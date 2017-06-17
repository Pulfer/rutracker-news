#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include "rutrackerfetcher.h"
#include "settingswindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    RutrackerFetcher fetcher;
    SettingsWindow *settingsWindow;
    QWidget *basicScreen;
    QSplitter *mainScreen;
    QWidget *loginScreen;
    QWidget *optionsScreen;
    QWidget *getTopicsWidget;
    QWidget *viewTopicWidget;
    QWidget *topicSummaryWidget;

    QStackedLayout *basicLayout;
    QVBoxLayout *getTopicsVLayout;
    QHBoxLayout *getTopicsHLayout;

    QVBoxLayout *viewTopicVLayout;

    QVBoxLayout *topicSummaryLayout;

    QHBoxLayout *topicSummarySubHLayout;
    QVBoxLayout *topicSummarySubVLayout;

    QHBoxLayout *ratingsLayout;

    QScrollArea *summaryScrollArea;

    QListWidget *topicsListWidget;
    QPushButton *getTopicsButton;

    QLabel *statusLabel;

    QAction *exitAction;
    QAction *settingsAction;

    QLabel *fullTitleLabel;
    QLabel *summaryLabel;
    QLabel *summaryExtraLabel;
    QLabel *descriptionLabel;

    QLabel *imageLabel;
    QLabel *imdbLabel;
    QLabel *kinopoiskLabel;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();  

private slots:
    void showSettings();
    void getTopics();
    void displayTopics(bool done);
    void showSummary(int topicIndex);

};

#endif // MAINWINDOW_H
