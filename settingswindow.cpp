#include "settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) : QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint);
    this->setWindowTitle(QString::fromUtf8("Параметры"));
    tabbedWidget = new QTabWidget;
    tabbedWidget->setUsesScrollButtons(false);
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabbedWidget);
    buttonsLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setStretch(0,1);
    mainLayout->setStretch(1,0);
    okButton = new QPushButton(QString::fromUtf8("ОК"));
    cancelButton = new QPushButton(QString::fromUtf8("Отмена"));
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    forumsScreen = new QWidget;
    loginScreen = new QWidget;

    tabbedWidget->addTab(forumsScreen, QString::fromUtf8("Разделы"));
    tabbedWidget->addTab(loginScreen, QString::fromUtf8("Аккаунт"));

    forumsLayout = new QHBoxLayout(forumsScreen);
    moviesGroup = new QGroupBox(QString::fromUtf8("Художественные"));
    cartoonsGroup = new QGroupBox(QString::fromUtf8("Мультфильмы"));
    documentaryGroup = new QGroupBox(QString::fromUtf8("Документальные"));
    moviesLayout = new QVBoxLayout(moviesGroup);
    cartoonsLayout = new QVBoxLayout(cartoonsGroup);
    documentaryLayout = new QVBoxLayout(documentaryGroup);

    latestMoviesCheck = new QCheckBox(QString::fromUtf8("Новейшие фильмы (2011 - 2015)"));
    newMoviesCheck = new QCheckBox(QString::fromUtf8("Зарубежные фильмы (до 2010)"));
    oldMoviesCheck = new QCheckBox(QString::fromUtf8("Классика зарубежного кино"));
    artHouseCheck = new QCheckBox(QString::fromUtf8("Арт-хаус и авторское кино"));
    asianCheck = new QCheckBox(QString::fromUtf8("Азиатские фильмы"));
    grindhouseCheck = new QCheckBox(QString::fromUtf8("Грайндхаус"));
    russianCheck = new QCheckBox(QString::fromUtf8("Наше кино"));
    ussrCheck = new QCheckBox(QString::fromUtf8("Кино СССР"));
    tvSeriesCheck = new QCheckBox(QString::fromUtf8("Зарубежные сериалы"));
    tvSeriesRussianCheck = new QCheckBox(QString::fromUtf8("Русские сериалы"));
    moviesLayout->addWidget(latestMoviesCheck);
    moviesLayout->addWidget(newMoviesCheck);
    moviesLayout->addWidget(oldMoviesCheck);
    moviesLayout->addWidget(artHouseCheck);
    moviesLayout->addWidget(asianCheck);
    moviesLayout->addWidget(grindhouseCheck);
    moviesLayout->addWidget(russianCheck);
    moviesLayout->addWidget(ussrCheck);
    moviesLayout->addWidget(tvSeriesCheck);
    moviesLayout->addWidget(tvSeriesRussianCheck);
    moviesLayout->addStretch(1);

    animeCheck = new QCheckBox(QString::fromUtf8("Аниме (основной и HD разделы)"));
    cartoonsCheck = new QCheckBox(QString::fromUtf8("Иностранные мультфильмы"));
    cartoonSeriesCheck = new QCheckBox(QString::fromUtf8("Мультсериалы"));
    russianCartoonsCheck = new QCheckBox(QString::fromUtf8("Отечественные мультфильмы"));
    cartoonsLayout->addWidget(animeCheck);
    cartoonsLayout->addWidget(cartoonsCheck);
    cartoonsLayout->addWidget(cartoonSeriesCheck);
    cartoonsLayout->addWidget(russianCartoonsCheck);
    cartoonsLayout->addStretch(1);

    criminalCheck = new QCheckBox(QString::fromUtf8("Криминальная документалистика"));
    bbcCheck = new QCheckBox(QString::fromUtf8("BBC"));
    discoveryCheck = new QCheckBox(QString::fromUtf8("Discovery"));
    ngCheck = new QCheckBox(QString::fromUtf8("National Geographic"));
    documentaryLayout->addWidget(criminalCheck);
    documentaryLayout->addWidget(bbcCheck);
    documentaryLayout->addWidget(discoveryCheck);
    documentaryLayout->addWidget(ngCheck);
    documentaryLayout->addStretch(1);

    forumsLayout->addWidget(moviesGroup);
    forumsLayout->addWidget(cartoonsGroup);
    forumsLayout->addWidget(documentaryGroup);
    moviesGroup->setMinimumWidth(300);
    cartoonsGroup->setMinimumWidth(300);
    documentaryGroup->setMinimumWidth(300);

    loginLayout = new QFormLayout(loginScreen);
    loginEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    loginLayout->addRow(QString::fromUtf8("Пользователь"), loginEdit);
    loginLayout->addRow(QString::fromUtf8("Пароль"), passwordEdit);

    connect(okButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "MIB", "rutracker-news");
}

void SettingsWindow::showEvent(QShowEvent *event)
{
    /* read settings */
    loginEdit->setText(settings->value("login").toString());
    passwordEdit->setText(settings->value("password").toString());
    latestMoviesCheck->setChecked(settings->value("latest-movies").toBool());
    newMoviesCheck->setChecked(settings->value("new-movies").toBool());
    oldMoviesCheck->setChecked(settings->value("old-movies").toBool());
    artHouseCheck->setChecked(settings->value("art-house").toBool());
    asianCheck->setChecked(settings->value("asian").toBool());
    grindhouseCheck->setChecked(settings->value("grindhouse").toBool());
    russianCheck->setChecked(settings->value("russian").toBool());
    ussrCheck->setChecked(settings->value("ussr").toBool());
    tvSeriesCheck->setChecked(settings->value("tv-series").toBool());
    tvSeriesRussianCheck->setChecked(settings->value("tv-series-russian").toBool());
    animeCheck->setChecked(settings->value("anime").toBool());
    cartoonsCheck->setChecked(settings->value("cartoons").toBool());
    cartoonSeriesCheck->setChecked(settings->value("cartoon-series").toBool());
    russianCartoonsCheck->setChecked(settings->value("russian-cartoons").toBool());
    criminalCheck->setChecked(settings->value("criminal").toBool());
    bbcCheck->setChecked(settings->value("bbc").toBool());
    discoveryCheck->setChecked(settings->value("discovery").toBool());
    ngCheck->setChecked(settings->value("ng").toBool());
}

void SettingsWindow::saveSettings()
{
    settings->setValue("login", loginEdit->text());
    settings->setValue("password", passwordEdit->text());
    settings->setValue("latest-movies", latestMoviesCheck->isChecked());
    settings->setValue("new-movies", newMoviesCheck->isChecked());
    settings->setValue("old-movies", oldMoviesCheck->isChecked());
    settings->setValue("art-house", artHouseCheck->isChecked());
    settings->setValue("asian", asianCheck->isChecked());
    settings->setValue("grindhouse", grindhouseCheck->isChecked());
    settings->setValue("russian", russianCheck->isChecked());
    settings->setValue("ussr", ussrCheck->isChecked());
    settings->setValue("tv-series", tvSeriesCheck->isChecked());
    settings->setValue("tv-series-russian", tvSeriesRussianCheck->isChecked());
    settings->setValue("anime", animeCheck->isChecked());
    settings->setValue("cartoons", cartoonsCheck->isChecked());
    settings->setValue("cartoon-series", cartoonSeriesCheck->isChecked());
    settings->setValue("russian-cartoons", russianCartoonsCheck->isChecked());
    settings->setValue("criminal", criminalCheck->isChecked());
    settings->setValue("bbc", bbcCheck->isChecked());
    settings->setValue("discovery", discoveryCheck->isChecked());
    settings->setValue("ng", ngCheck->isChecked());

    settings->sync();
    accept();
}

SettingsWindow::~SettingsWindow()
{
    delete settings;
}
