#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedLayout>
#include <QTabWidget>
#include <QVBoxLayout>

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

private:
    QTabWidget *tabbedWidget;
    QWidget *forumsScreen;
    QWidget *loginScreen;
    QWidget *proxyScreen;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QHBoxLayout *forumsLayout;
    QGroupBox *moviesGroup;
    QGroupBox *cartoonsGroup;
    QGroupBox *documentaryGroup;
    QVBoxLayout *moviesLayout;
    QVBoxLayout *cartoonsLayout;
    QVBoxLayout *documentaryLayout;

    QCheckBox *latestMoviesCheck;
    QCheckBox *newMoviesCheck;
    QCheckBox *oldMoviesCheck;
    QCheckBox *artHouseCheck;
    QCheckBox *asianCheck;
    QCheckBox *grindhouseCheck;
    QCheckBox *russianCheck;
    QCheckBox *ussrCheck;
    QCheckBox *tvSeriesCheck;
    QCheckBox *tvSeriesRussianCheck;

    QCheckBox *animeCheck;
    QCheckBox *cartoonsCheck;
    QCheckBox *cartoonSeriesCheck;
    QCheckBox *russianCartoonsCheck;

    QCheckBox *criminalCheck;
    QCheckBox *bbcCheck;
    QCheckBox *discoveryCheck;
    QCheckBox *ngCheck;

    QFormLayout *loginLayout;
    QLineEdit *loginEdit;
    QLineEdit *passwordEdit;

    QFormLayout *proxyLayout;
    QCheckBox *proxyCheck;
    QLineEdit *proxyUrlEdit;
    QSpinBox *proxyPortBox;
    QComboBox *proxyTypeBox;
    QLineEdit *proxyLoginEdit;
    QLineEdit *proxyPasswordEdit;

    QSettings *settings;

protected:
    void showEvent(QShowEvent *event);

signals:

private slots:
    void toggleProxy(int state);
    void saveSettings();

};

#endif // SETTINGSWINDOW_H
