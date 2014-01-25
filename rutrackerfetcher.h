#ifndef RUTRACKERFETCHER_H
#define RUTRACKERFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QtNetwork>

struct topic {
    QString url;
    QString forum;
    QString fullTitle;
    QString title;
    QString season;
    QString episodes;
    QString year;
    QString country;
    QString genre;
    QString duration;
    QString translation;
    QString director;
    QString imageUrl;
    QString quality;
    QString format;
    QString video;
    QString audio1;
    QString audio2;
    QString description;
    QByteArray image;
    QString imdb;
};

struct parser {
    int forumNumber;
    QString sectionEnd;
    QString yearBegin;
    QString countryBegin;
    QString genreBegin;
    QString durationBegin;
    QString translationBegin;
    QString directorBegin;
    QString imageUrlBegin;
    QString imageUrlEnd;
    QString qualityBegin;
    QString formatBegin;
    QString videoBegin;
    QString audioBegin;
    QString descriptionBegin;
    QString descriptionEnd;
};

class RutrackerFetcher : public QObject
{
    Q_OBJECT
public:
    RutrackerFetcher(QObject *parent = 0);
    ~RutrackerFetcher();
    void fetch();
    int count();
    void setMaxTopics(int max);
    topic at(int topicIndex);

private:
    QList<topic> topicsList;
    QStringList searchList;

    QNetworkAccessManager qnam;
    QNetworkReply *reply;

    int currentPage;
    int pagesCount;
    int currentTopic;
    int maxTopics;

    void setupParsers();
    void getNextTopic();
    void parseNextTopic();
    void getNextImage();
    void getNextImdb();
    void parseForum(QString postBody, parser postParser);
    QString fixString(QString stringToFix);
    QString fixTitle(QString stringToFix);
    QString removeSpans(QString stringToFix);

    parser parser1389;

    QString basicUrl;
    bool fetching;
    QSettings *settings;

    QString accountLogin;
    QString accountPassword;

    QDir imageDir;

signals:
    void stateChanged(QString newState);
    void finished(bool done);
    void loggedIn();
    void gotTopicList();
    void parsedTopics();
    void gotImdb();
    void gotImages();

private slots:
    void login();
    void getTopics();
    void parseTopics();
    void getImages();
    void getImdb();
    void loginFinished(QNetworkReply*);
    void getTopicFinished(QNetworkReply *topicsReply);
    void parseTopicFinished(QNetworkReply *topicsReply);
    void getImageFinished(QNetworkReply *topicsReply);
    void getImdbFinished(QNetworkReply *topicsReply);
};

#endif // RUTRACKERFETCHER_H
