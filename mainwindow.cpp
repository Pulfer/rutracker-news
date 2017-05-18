#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Rutracker.org news 0.7");
    setMinimumSize(800, 600);
    basicScreen = new QWidget;
    setCentralWidget(basicScreen);

    exitAction = new QAction(QString::fromUtf8("Выход"), this);
    settingsAction = new QAction(QString::fromUtf8("Параметры"), this);

    menuBar()->addMenu(QString::fromUtf8("Файл"))->addAction(exitAction);
    menuBar()->addMenu(QString::fromUtf8("Настройки"))->addAction(settingsAction);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));

    basicLayout = new QStackedLayout(basicScreen);
    mainScreen = new QSplitter(basicScreen);
    loginScreen = new QWidget(basicScreen);
    optionsScreen = new QWidget(basicScreen);
    basicLayout->addWidget(mainScreen);
    basicLayout->addWidget(loginScreen);
    basicLayout->addWidget(optionsScreen);

    getTopicsWidget = new QWidget;
    topicsListWidget = new QListWidget;
    getTopicsButton = new QPushButton(QString::fromUtf8("Проверить обновления"));
    getTopicsVLayout = new QVBoxLayout(getTopicsWidget);
    getTopicsHLayout = new QHBoxLayout;
    getTopicsHLayout->addWidget(getTopicsButton);
    getTopicsVLayout->addWidget(topicsListWidget);
    getTopicsVLayout->addLayout(getTopicsHLayout);

    viewTopicWidget = new QWidget;

    summaryScrollArea = new QScrollArea;
    topicSummaryWidget = new QWidget;
    topicSummaryLayout = new QVBoxLayout(topicSummaryWidget);
    topicSummarySubHLayout = new QHBoxLayout;
    summaryScrollArea->setWidget(topicSummaryWidget);
    summaryScrollArea->setWidgetResizable(true);

    viewTopicVLayout = new QVBoxLayout(viewTopicWidget);
    viewTopicVLayout->addWidget(summaryScrollArea);

    mainScreen->addWidget(getTopicsWidget);
    mainScreen->addWidget(viewTopicWidget);
    mainScreen->setStretchFactor(0, 2);
    mainScreen->setStretchFactor(1, 6);

    fullTitleLabel = new QLabel;
    fullTitleLabel->setText(QString::fromUtf8("<h2>Ничего не выбрано</h2>"));
    fullTitleLabel->setWordWrap(true);
    summaryLabel = new QLabel;
    summaryLabel->setText(QString::fromUtf8("<b>Нажмите на \"Проверить обновления\"</b> "));
    summaryLabel->setWordWrap(true);
    summaryLabel->setOpenExternalLinks(true);
    summaryLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    summaryLabel->setAlignment(Qt::AlignTop);

    descriptionLabel = new QLabel;
    descriptionLabel->setWordWrap(true);

    imageLabel = new QLabel;
    imageLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    topicSummaryLayout->addWidget(fullTitleLabel); 

    topicSummaryLayout->addLayout(topicSummarySubHLayout);
    topicSummarySubHLayout->addWidget(summaryLabel);
    topicSummarySubHLayout->addWidget(imageLabel);
    topicSummarySubHLayout->setStretch(0, 1);

    topicSummaryLayout->addWidget(descriptionLabel);

    topicSummaryLayout->addStretch(1);
    basicLayout->setCurrentIndex(0);

    statusLabel = new QLabel;
    statusLabel->setText(QString::fromUtf8("Готово"));
    statusBar()->addWidget(statusLabel);
    statusBar()->setSizeGripEnabled(false);
    statusBar()->setStyleSheet("QStatusBar {border-top: 1px solid black} QStatusBar::item {border: 0px solid black};");
    connect(&fetcher, SIGNAL(stateChanged(QString)), statusLabel, SLOT(setText(QString)));
    connect(&fetcher, SIGNAL(finished(bool)), this, SLOT(displayTopics(bool)));
    connect(getTopicsButton, SIGNAL(clicked()), this, SLOT(getTopics()));
    connect(topicsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showSummary(int)));

    settingsWindow = new SettingsWindow(this);
    settingsWindow->setModal(true);
}

void MainWindow::showSettings()
{
    settingsWindow->show();
    settingsWindow->move(x() + width()/2 - settingsWindow->width()/2, y() + height()/2 - settingsWindow->height()/2);
}

void MainWindow::getTopics()
{
    getTopicsButton->setEnabled(false);
    topicsListWidget->setEnabled(false);
    fetcher.setMaxTopics(40);
    fetcher.fetch();
}

void MainWindow::displayTopics(bool done)
{
    getTopicsButton->setEnabled(true);
    topicsListWidget->setEnabled(true);
    topicsListWidget->clear();
    topicsListWidget->setCurrentRow(-1);
    if (done)
    {
        statusLabel->setText(QString::fromUtf8("Готово. Отобрано раздач: ") + QString::number(fetcher.count()));
        for (int i = 0; i < fetcher.count(); i++)
        {
            topicsListWidget->addItem(fetcher.at(i).title);
        }
        topicsListWidget->setCurrentRow(0);
    }
}

void MainWindow::showSummary(int topicIndex)
{
    if (topicIndex < 0)
    {
        imageLabel->clear();
        fullTitleLabel->setText(QString::fromUtf8("<h2>Ничего не выбрано</h2>"));
        summaryLabel->setText(QString::fromUtf8("<b>Нажмите на \"Проверить обновления\"</b> "));
        descriptionLabel->setText("");
    }
    else
    {
        QImage image;
        image.loadFromData(fetcher.at(topicIndex).image);
        imageLabel->setPixmap(QPixmap::fromImage(image).scaled(250, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QString summary;

        fullTitleLabel->setText("<h2>" + fetcher.at(topicIndex).fullTitle + "</h2>");

        summary = QString::fromUtf8("<b>Название:</b> ") + fetcher.at(topicIndex).title;
        if (!fetcher.at(topicIndex).season.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Сезон:</b> ") + fetcher.at(topicIndex).season;
        }
        if (!fetcher.at(topicIndex).episodes.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Серии:</b> ") + fetcher.at(topicIndex).episodes;
        }
        if (!fetcher.at(topicIndex).year.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Год:</b> ") + fetcher.at(topicIndex).year;
        }
        if (!fetcher.at(topicIndex).country.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Страна:</b> ") + fetcher.at(topicIndex).country;
        }
        if (!fetcher.at(topicIndex).genre.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Жанр:</b> ") + fetcher.at(topicIndex).genre;
        }
        if (!fetcher.at(topicIndex).director.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Режиссер:</b> ") + fetcher.at(topicIndex).director;
        }
        if (!fetcher.at(topicIndex).duration.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Продолжительность:</b> ") + fetcher.at(topicIndex).duration;
        }
        if (!fetcher.at(topicIndex).translation.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Перевод:</b> ") + fetcher.at(topicIndex).translation;
        }
        if (!fetcher.at(topicIndex).imdb.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Рейтинг IMDB:</b> ") + fetcher.at(topicIndex).imdb;
        }
        if (!fetcher.at(topicIndex).quality.isEmpty())
        {
            summary = summary + QString::fromUtf8("<hr><b>Качество:</b> ") + fetcher.at(topicIndex).quality;
        }
        else
        {
            summary = summary + "<hr>";
        }
        if (!fetcher.at(topicIndex).format.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Формат:</b> ") + fetcher.at(topicIndex).format;
        }
        if (!fetcher.at(topicIndex).video.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Видео:</b> ") + fetcher.at(topicIndex).video;
        }
        if (!fetcher.at(topicIndex).audio1.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Аудио:</b> ") + fetcher.at(topicIndex).audio1;
        }
        if (!fetcher.at(topicIndex).audio2.isEmpty())
        {
            summary = summary + QString::fromUtf8("<br><b>Аудио 2:</b> ") + fetcher.at(topicIndex).audio2;
        }
        summary = summary + QString::fromUtf8("<hr><b>Раздел:</b> ") + fetcher.at(topicIndex).forum;
        summary = summary + QString::fromUtf8("<br><b>Тема:</b> <a href=\"") + fetcher.at(topicIndex).url + "\">" + fetcher.at(topicIndex).url + "</a>";
        summaryLabel->setText(summary);

        descriptionLabel->setText(QString::fromUtf8("<hr><b>Описание:</b> ") + fetcher.at(topicIndex).description);
    }
}

MainWindow::~MainWindow()
{
}
