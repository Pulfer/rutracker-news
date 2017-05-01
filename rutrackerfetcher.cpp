#include "rutrackerfetcher.h"
#include <QDebug>
#ifdef Q_OS_WIN
#include "windows.h"
#endif

RutrackerFetcher::RutrackerFetcher(QObject *parent) : QObject(parent)
{
    #ifdef Q_OS_WIN
    imageDir = QDir::homePath() + "/Application Data/MIB/images/";
    #else
    imageDir = QDir::homePath() + "/.config/MIB/images/";
    #endif

    if (!imageDir.exists())
    {
        QDir().mkpath(imageDir.path());
    }
    maxTopics = 1000;
    fetching = false;
    setupParsers();
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "MIB", "rutracker-news");
    connect(this, SIGNAL(loggedIn()), this, SLOT(getTopics()));
    connect(this, SIGNAL(gotTopicList()), this, SLOT(parseTopics()));
    connect(this, SIGNAL(parsedTopics()), this, SLOT(getImdb()));
    connect(this, SIGNAL(gotImdb()), this, SLOT(getImages()));
}

void qSleep(int ms)
{
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void qWait(int ms)
{
    Q_ASSERT(QCoreApplication::instance());
    QTime timer;
    timer.start();
    do {
        QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
        qSleep(10);
    } while (timer.elapsed() < ms);
}

void RutrackerFetcher::setupParsers()
{
    parser1389.forumNumber = 1389;
    parser1389.sectionEnd = "<br />";
    parser1389.yearBegin = QString::fromUtf8("!section!Год выпуска:");
    parser1389.countryBegin = QString::fromUtf8("!section!Страна:");
    parser1389.genreBegin = QString::fromUtf8("!section!Жанр:");
    parser1389.durationBegin = QString::fromUtf8("!section!Продолжительность:");
    parser1389.translationBegin = QString::fromUtf8("!section!Перевод:");
    parser1389.directorBegin = QString::fromUtf8("!section!Режиссер:");
    parser1389.imageUrlBegin = QString::fromUtf8("!image!");
    parser1389.imageUrlEnd = QString::fromUtf8("\">");
    parser1389.qualityBegin = QString::fromUtf8("!section!Качество:");
    parser1389.formatBegin = QString::fromUtf8("!section!Формат:");
    parser1389.videoBegin = QString::fromUtf8("!section!Видео:");
    parser1389.audioBegin = QString::fromUtf8("!section!Аудио:");
    parser1389.descriptionBegin = QString::fromUtf8("!section!Описание:");
    parser1389.descriptionEnd = QString::fromUtf8("!section!");
}

int RutrackerFetcher::count()
{
    if (topicsList.count() < maxTopics)
    {
        return topicsList.count();
    }
    else
    {
        return maxTopics;
    }
}

void RutrackerFetcher::setMaxTopics(int max)
{
    maxTopics = max;
}

topic RutrackerFetcher::at(int topicIndex)
{
    if ((topicIndex >= 0) && (topicIndex <= this->count()))
    {
        return topicsList.at(topicIndex);
    }
    else
    {
        return topicsList.at(0);
    }
}


void RutrackerFetcher::fetch()
{
    if (!fetching)
    {
        accountLogin = settings->value("login").toString();
        accountPassword = settings->value("password").toString();
        if (accountLogin.isEmpty() || accountPassword.isEmpty())
        {
            emit stateChanged(QString::fromUtf8("Укажите имя пользователя и пароль!"));
        }
        else
        {
            QString url = "";
            if (settings->value("latest-movies").toBool())
            {
                url = url + "&f[]=2093&f[]=2200";
            }
            if (settings->value("new-movies").toBool())
            {
                url = url + "&f[]=2090&f[]=2091&f[]=2092&f[]=2221";
            }
            if (settings->value("old-movies").toBool())
            {
                url = url + "&f[]=187";
            }
            if (settings->value("art-house").toBool())
            {
                url = url + "&f[]=124";
            }
            if (settings->value("asian").toBool())
            {
                url = url + "&f[]=934";
            }
            if (settings->value("grindhouse").toBool())
            {
                url = url + "&f[]=1235";
            }
            if (settings->value("russian").toBool())
            {
                url = url + "&f[]=22";
            }
            if (settings->value("ussr").toBool())
            {
                url = url + "&f[]=941";
            }
            if (settings->value("tv-series").toBool())
            {
                url = url + "&f[]=189";
            }
            if (settings->value("tv-series-russian").toBool())
            {
                url = url + "&f[]=9";
            }
            if (settings->value("anime").toBool())
            {
                url = url + "&f[]=1389&f[]=1105";
            }
            if (settings->value("cartoons").toBool())
            {
                url = url + "&f[]=209";
            }
            if (settings->value("cartoon-series").toBool())
            {
                url = url + "&f[]=921";
            }
            if (settings->value("russian-cartoons").toBool())
            {
                url = url + "&f[]=208";
            }
            if (settings->value("criminal").toBool())
            {
                url = url + "&f[]=251";
            }
            if (settings->value("bbc").toBool())
            {
                url = url + "&f[]=249";
            }
            if (settings->value("discovery").toBool())
            {
                url = url + "&f[]=552";
            }
            if (settings->value("ng").toBool())
            {
                url = url + "&f[]=500";
            }


            if (url.isEmpty())
            {
                emit stateChanged(QString::fromUtf8("Не выбран ни один раздел!"));
                emit finished(false);
            }
            else
            {
                fetching = true;
                searchList.clear();
                url = "http://rutracker.org/forum/tracker.php?oop=1&sd=1&tm=3" + url;
                searchList << url;
                basicUrl = url;
                login();
            }
        }
    }
}

void RutrackerFetcher::login()
{
    emit stateChanged(QString::fromUtf8("Логинимся"));
    if (settings->value("use-proxy").toBool())
    {
        QNetworkProxy proxy;
        if (settings->value("proxy-type").toString() == "HTTP")
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        else
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        proxy.setHostName(settings->value("proxy-url").toString());
        proxy.setPort(settings->value("proxy-port").toInt());
        QString proxyLogin = settings->value("proxy-login").toString();
        if (!proxyLogin.isEmpty())
        {
            proxy.setUser(proxyLogin);
        }
        QString proxyPassword = settings->value("proxy-password").toString();
        if (!proxyPassword.isEmpty())
        {
            proxy.setPassword(proxyPassword);
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }
    qnam.setCookieJar(new QNetworkCookieJar);
    QNetworkRequest loginRequest(QUrl("http://rutracker.org/forum/login.php"));
    loginRequest.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QByteArray postData;
    postData.append("login_username=" + accountLogin);
    postData.append("&login_password=" + accountPassword);
    postData.append("&login=%C2%F5%EE%E4");
    qnam.post(loginRequest, postData);
    connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(loginFinished(QNetworkReply*)));
}

void RutrackerFetcher::loginFinished(QNetworkReply *loginReply)
{
    if (!loginReply->error())
    {
        QString temp;
        temp = loginReply->readAll();
        if (temp.contains(QString::fromUtf8("код подтверждения")))
        {
            emit stateChanged(QString::fromUtf8("Не введён код подтверждения"));
        }
        else
        {
            emit stateChanged(QString::fromUtf8("Залогинились"));
            emit loggedIn();
        }
    }
    else
    {
            emit stateChanged(QString::fromUtf8("Неизвестная ошибка при входе"));
    }
    disconnect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(loginFinished(QNetworkReply*)));
}

void RutrackerFetcher::getTopics()
{
    topicsList.clear();
    currentPage = 0;
    pagesCount = 0;
    getNextTopic();
}

void RutrackerFetcher::getNextTopic()
{
    if (!searchList.empty())
    {
        QUrl url = QUrl(searchList.takeFirst());
        QNetworkRequest req(url);
        qnam.get(req);
        connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getTopicFinished(QNetworkReply*)));
    }
    else
    {
        emit stateChanged(QString::fromUtf8("Пустой запрос"));
        emit finished(false);
    }
}

void RutrackerFetcher::getTopicFinished(QNetworkReply *topicsReply)
{
    disconnect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getTopicFinished(QNetworkReply*)));
    if (!topicsReply->error())
    {
        QString temp = QString::fromLocal8Bit(topicsReply->readAll());
        if (temp.contains(QString::fromUtf8("форум временно отключен")))
        {
            emit stateChanged(QString::fromUtf8("Форум временно отключен"));
            fetching = false;
            emit finished(true);
        }
        if (temp.contains(QString::fromUtf8("Результатов поиска:")))
        {
            int startPos = temp.indexOf(QString::fromUtf8("Результатов поиска: ")) + 20;
            int endPos = temp.indexOf(" <span class=\"normal\">(max: 500)", startPos);
            int topicsFound = temp.mid(startPos, endPos - startPos).toInt();
            emit stateChanged(QString::fromUtf8("Найдено топиков: %1").arg(topicsFound));
            if (!currentPage)
            {
                pagesCount =  topicsFound / 50;
                if (!(topicsFound % 50) && !(pagesCount == 0))
                {
                    pagesCount--;
                }
                for (int i = 1; i <= pagesCount; i++)
                {
                    searchList << basicUrl + "&start=" + QString::number(i*50);
                }
            }
            if (topicsFound > 0)
                startPos = temp.indexOf("search-results");
            else
                startPos = 0;
            int endSearch = temp.indexOf("/search-results");
            topic tempTopic;
            while (startPos > 0)
            {
                startPos = temp.indexOf("viewtopic.php?t=", startPos + 16);
                if ((startPos > 0) && (startPos < endSearch))
                {
                    startPos = startPos + 16;
                    endPos = temp.indexOf("\">", startPos);
                    tempTopic.url = "http://rutracker.org/forum/viewtopic.php?t=" + temp.mid(startPos, endPos - startPos);
                    tempTopic.title = "None";
                    topicsList.append(tempTopic);
                }
            }
        }
        topicsReply->deleteLater();
        if (currentPage < pagesCount)
        {
            currentPage++;
            getNextTopic();
        }
        else
        {
            emit gotTopicList();
        }
    }
    else
    {
        qDebug() << topicsReply->errorString();
        topicsReply->deleteLater();
        emit stateChanged(QString::fromUtf8("Ошибка при анализе результатов поиска"));
        emit finished(false);
    }
}

void RutrackerFetcher::getImages()
{
    currentTopic = 0;
    if (!topicsList.empty())
    {
        getNextImage();
    }
    else
    {
        emit finished(false);
        fetching = false;
    }
}

void RutrackerFetcher::getNextImage()
{
    if (currentTopic > count() - 1)
    {
        emit stateChanged(QString::fromUtf8("Обложки загружены"));
        emit finished(true);
        fetching = false;
    }
    else
    {
        emit stateChanged(QString::fromUtf8("Загрузка обложки: ") + QString::number(currentTopic + 1));
        QFile img(imageDir.path() + "/" + QCryptographicHash::hash(QByteArray(topicsList.at(currentTopic).title.toLocal8Bit()), QCryptographicHash::Md5).toHex());
        if (img.exists())
        {
            img.open(QIODevice::ReadOnly);
            topicsList[currentTopic].image = img.readAll();
            img.close();
            currentTopic++;
            getNextImage();
        }
        else
        {
            qWait(1000);
            QNetworkRequest req(QUrl(topicsList.at(currentTopic).imageUrl));
            qnam.get(req);
            connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getImageFinished(QNetworkReply*)));
        }
    }
}

void RutrackerFetcher::getImageFinished(QNetworkReply *topicsReply)
{
    disconnect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getImageFinished(QNetworkReply*)));
    topicsReply->deleteLater();
    bool needFallback = false;
    if (!topicsReply->error())
    {
        topicsList[currentTopic].image = topicsReply->readAll();
        /* skip 16+ and 18+ images, they are likely to be less than 20000 bytes */
        if (topicsList.at(currentTopic).image.size() > 20000)
        {
            QFile img(imageDir.path() + "/" + QCryptographicHash::hash(QByteArray(topicsList.at(currentTopic).title.toLocal8Bit()), QCryptographicHash::Md5).toHex());
            img.open(QIODevice::WriteOnly);
            img.write(topicsList.at(currentTopic).image);
            img.close();
        }
        else if (!topicsList.at(currentTopic).imageUrlFallback.isEmpty())
        {
            needFallback = true;
            topicsList[currentTopic].imageUrl = topicsList.at(currentTopic).imageUrlFallback;
            topicsList[currentTopic].imageUrlFallback.clear();
        }
    }
    else
    {
        emit stateChanged(QString::fromUtf8("Ошибка загрузки обложки: ") + QString::number(currentTopic + 1));
        qDebug() << topicsReply->errorString();
    }
    if (!needFallback)
        currentTopic++;
    getNextImage();
}

void RutrackerFetcher::getImdb()
{
    /* don't touch proxy settings unless we use enable it in options */
    if (settings->value("use-proxy").toBool())
    {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
    currentTopic = 0;
    if (!topicsList.empty())
        getNextImdb();
    else
        emit gotImdb();
}

void RutrackerFetcher::getNextImdb()
{
    if (currentTopic > count() - 1)
    {
        emit stateChanged(QString::fromUtf8("Поиск рейтингов IMDB завершён"));
        emit gotImdb();
    }
    else
    {
        qWait(250);
        emit stateChanged(QString::fromUtf8("Загрузка рейтинга IMDB: ") + QString::number(currentTopic + 1));
        QString title = topicsList.at(currentTopic).title;
        title.remove(0, title.lastIndexOf("/") + 1);
        title = title.simplified();
        QUrl tempUrl;
        if (topicsList.at(currentTopic).year.length() == 4)
            tempUrl.setUrl("http://www.omdbapi.com/?t=" + title + "&y=" + topicsList.at(currentTopic).year);
        else
            tempUrl.setUrl("http://www.omdbapi.com/?t=" + title);
        QNetworkRequest req(tempUrl);
        qnam.get(req);
        connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getImdbFinished(QNetworkReply*)));
    }
}

void RutrackerFetcher::getImdbFinished(QNetworkReply *topicsReply)
{
    disconnect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(getImdbFinished(QNetworkReply*)));
    topicsReply->deleteLater();
    if (!topicsReply->error())
    {
        QString temp;
        temp = topicsReply->readAll();
        int startPos = temp.indexOf(QString::fromUtf8("\"imdbRating\":\""));
        if (startPos > 0)
        {
            startPos = startPos + 14;
            int endPos = temp.indexOf("\"", startPos);
            topicsList[currentTopic].imdb = temp.mid(startPos, endPos - startPos);
        }
    }
    else
    {
        emit stateChanged(QString::fromUtf8("Ошибка загрузки рейтинга IMDB: ") + QString::number(currentTopic) + 1);
        qDebug() << topicsReply->errorString();
    }
    currentTopic++;
    getNextImdb();
}

void RutrackerFetcher::parseTopics()
{
    currentTopic = 0;
    if (!topicsList.empty())
        parseNextTopic();
    else
        emit parsedTopics();
}

void RutrackerFetcher::parseNextTopic()
{
    if (currentTopic > count() - 1)
    {
        emit stateChanged(QString::fromUtf8("Анализ раздач завершён"));
        emit parsedTopics();
    }
    else
    {
        qWait(250);
        emit stateChanged(QString::fromUtf8("Анализ раздачи: ") + QString::number(currentTopic + 1));
        QUrl tempUrl = QUrl(topicsList.at(currentTopic).url);
        QNetworkRequest req(tempUrl);
        qnam.get(req);
        connect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseTopicFinished(QNetworkReply*)));
    }
}

void RutrackerFetcher::parseTopicFinished(QNetworkReply *topicsReply)
{
    disconnect(&qnam, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseTopicFinished(QNetworkReply*)));
    topicsReply->deleteLater();
    if (!topicsReply->error())
    {
        QString temp = QString::fromLocal8Bit(topicsReply->readAll());
        int startPos = temp.indexOf(QString::fromUtf8("<title>")) + 7;
        int endPos = temp.indexOf("</title>", startPos);
        topicsList[currentTopic].fullTitle = temp.mid(startPos, endPos - startPos);
        topicsList[currentTopic].fullTitle.remove(" :: RuTracker.org");
        startPos = temp.indexOf(QString::fromUtf8("RuTracker.org &raquo; "));
        startPos = temp.indexOf(QString::fromUtf8("&raquo;"), startPos);
        startPos = temp.indexOf(QString::fromUtf8(" "), startPos);
        endPos = temp.indexOf(QString::fromUtf8("&raquo; Скачать"), startPos);
        topicsList[currentTopic].forum = fixString(temp.mid(startPos, endPos - startPos));
        startPos = temp.indexOf(QString::fromUtf8("<div class=\"post_body\""));
        endPos = temp.indexOf(QString::fromUtf8("<!--/post_body-->"));
        temp = temp.mid(startPos, endPos - startPos);
        parseForum(temp, parser1389);
    }
    else
    {
        emit stateChanged(QString::fromUtf8("Ошибка при анализе раздачи: ") + QString::number(currentTopic + 1));
        qDebug() << topicsReply->errorString();
    }
    currentTopic++;
    parseNextTopic();
}

QString RutrackerFetcher::fixString(QString stringToFix)
{
    stringToFix.replace("<br />", " ");
    stringToFix.replace("&raquo;", ">");
    stringToFix.replace("&nbsp;", " ");
    stringToFix.remove(QRegExp("<[^>]*>"));
    stringToFix.remove(QRegExp("&[^;]*;"));
    stringToFix.remove("!section!");
    stringToFix = stringToFix.simplified();
    return stringToFix;
}

QString RutrackerFetcher::removeSpans(QString stringToFix)
{
    stringToFix.replace("<span class=\"post-b\">", "!section!");
    stringToFix.replace("<span class=\"post-hr\">", "<br />");
    stringToFix.replace("<var class=\"postImg postImgAligned img-right\" title=\"", "!image!");
    stringToFix.replace("<var class=\"postImg postImgAligned img-left\" title=\"", "!image!");
    stringToFix.replace("</div>", "<br />");
    stringToFix.remove("<ul>");
    stringToFix.remove("<li>");
    stringToFix.remove("</li>");
    stringToFix.replace("</ul>", "<br />");

    stringToFix.replace("<span class=\"post-align\" style=\"text-align: center;\">", "<br />");
    stringToFix.remove(QRegExp("<span[^>]*>"));
    stringToFix.remove("</span>");

    stringToFix.replace("/ !section!", "<br /> !section!");
    stringToFix.replace("| !section!", "<br /> !section!");
    stringToFix.replace(", !section!", "<br /> !section!");

    stringToFix.replace("<div class=\"sp-wrap\">", "!section!<br />");

    /* hacks */
    stringToFix.replace(QString::fromUtf8("Год:"), QString::fromUtf8("Год выпуска:"));
    stringToFix.replace(QString::fromUtf8("Режиссёр:"), QString::fromUtf8("Режиссер:"));
    stringToFix.replace(QString::fromUtf8("Качество видео:"), QString::fromUtf8("Качество:"));
    stringToFix.replace(QString::fromUtf8("Качество :"), QString::fromUtf8("Качество:"));
    stringToFix.replace(QString::fromUtf8("Контейнер:"), QString::fromUtf8("Формат:"));
    stringToFix.replace(QString::fromUtf8("Формат видео"), QString::fromUtf8("Формат"));
    stringToFix.replace(QString::fromUtf8("/ Тип видео:"), QString::fromUtf8("<br /> !section!Тип видео:"));
    stringToFix.replace(QString::fromUtf8("| Тип видео:"), QString::fromUtf8("<br /> !section!Тип видео:"));
    stringToFix.replace(QString::fromUtf8("/ Формат:"), QString::fromUtf8("<br /> !section!Формат:"));
    stringToFix.replace(QString::fromUtf8("| Формат:"), QString::fromUtf8("<br /> !section!Формат:"));
    stringToFix.replace(QString::fromUtf8(", Формат:"), QString::fromUtf8("<br /> !section!Формат:"));
    stringToFix.replace(QString::fromUtf8("Video"), QString::fromUtf8("Видео"));
    stringToFix.replace(QString::fromUtf8("Audio"), QString::fromUtf8("Аудио"));
    stringToFix.replace(QString::fromUtf8("Аудио 1:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио 2:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио 01:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио 02:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио №1:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио №2:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио #1"), QString::fromUtf8("Аудио"));
    stringToFix.replace(QString::fromUtf8("Аудио #2"), QString::fromUtf8("Аудио"));
    stringToFix.replace(QString::fromUtf8("Аудио#1:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио#2:"), QString::fromUtf8("Аудио:"));
    stringToFix.replace(QString::fromUtf8("Аудио, "), QString::fromUtf8("Аудио: "));
    stringToFix.replace(QString::fromUtf8("Аудио :"), QString::fromUtf8("Аудио: "));
    stringToFix.replace(QString::fromUtf8("Аудио JAP:"), QString::fromUtf8("Аудио: [JAP]"));
    stringToFix.replace(QString::fromUtf8("Аудио RUS:"), QString::fromUtf8("Аудио: [RUS]"));
    stringToFix.replace(QString::fromUtf8("Аудио RUS 1:"), QString::fromUtf8("Аудио: [RUS]"));
    stringToFix.replace(QString::fromUtf8("Аудио RUS 2:"), QString::fromUtf8("Аудио: [RUS]"));
    stringToFix.replace(QString::fromUtf8("О фильме:"), QString::fromUtf8("Описание:"));
    if (!stringToFix.contains(QString::fromUtf8("Описание:")) && stringToFix.contains(QString::fromUtf8("Описание")))
        stringToFix.replace(QString::fromUtf8("Описание"), QString::fromUtf8("Описание:"));

    stringToFix.remove(QRegExp("&[^;]*;"));
    stringToFix = stringToFix.simplified();
    return stringToFix;
}

QString RutrackerFetcher::fixTitle(QString stringToFix)
{
    int posBr = stringToFix.indexOf("[");
    if (posBr == 0)
    {
        posBr = stringToFix.indexOf("]") + 1;
        stringToFix = stringToFix.right(stringToFix.length() - posBr);
    }
    posBr = stringToFix.indexOf("]");
    int posSl = stringToFix.indexOf(" / ");
    if (posBr < posSl)
    {
        posBr = stringToFix.indexOf("]", posSl);
    }
    if (posBr > 0)
    {
        stringToFix = stringToFix.left(posBr + 1);
    }
    stringToFix.replace(QString("["), QString("<"));
    stringToFix.replace(QString("]"), QString(">"));
    stringToFix.remove(QRegExp("<[^>]*>"));
    stringToFix.replace(QString("("), QString("<"));
    stringToFix.replace(QString(")"), QString(">"));
    stringToFix.remove(QRegExp("<[^>]*>"));
    stringToFix.remove(QRegExp(QString::fromUtf8("/ Сезон[ы]*[:]* .*")));
    stringToFix.remove(QRegExp(QString::fromUtf8("/ Сери[и]*[я]*[:]* .*")));
    stringToFix = stringToFix.simplified();
    return stringToFix;
}

void RutrackerFetcher::parseForum(QString postBody, parser postParser)
{
    int startPos;
    int endPos;
    postBody = removeSpans(postBody);
    topicsList[currentTopic].fullTitle = fixString(topicsList.at(currentTopic).fullTitle);

    /* season */
    QRegExp seasonRegExp(QString::fromUtf8("/ Сезон[ы]*[:]* [0-9\,\ \-]+"));
    seasonRegExp.indexIn(topicsList.at(currentTopic).fullTitle);
    topicsList[currentTopic].season = seasonRegExp.cap(0);
    topicsList[currentTopic].season.remove(QRegExp(QString::fromUtf8("/ Сезон[ы]*[:]* ")));

    /* episodes */
    QRegExp episodesRegExp(QString::fromUtf8("\\[[0-9\,\ \-]+( из )[0-9\,\ \-]+\\]"));
    episodesRegExp.indexIn(topicsList.at(currentTopic).fullTitle);
    topicsList[currentTopic].episodes = episodesRegExp.cap(0);
    topicsList[currentTopic].episodes.remove("[");
    topicsList[currentTopic].episodes.remove("]");
    // Try another pattern if nothing is found
    if (topicsList[currentTopic].episodes.isEmpty())
    {
        episodesRegExp.setPattern(QString::fromUtf8("Сери[и]*[я]*[:]* [0-9\,\ \-]+( из )[0-9\,\ \-]+"));
        episodesRegExp.indexIn(topicsList.at(currentTopic).fullTitle);
        topicsList[currentTopic].episodes = episodesRegExp.cap(0);
        topicsList[currentTopic].episodes.remove(QRegExp(QString::fromUtf8("Сери[и]*[я]*[:]* ")));
    }
    // And one more pattern if nothing is found
    if (topicsList[currentTopic].episodes.isEmpty())
    {
        episodesRegExp.setPattern(QString::fromUtf8("Сери[и]*[я]*[:]* [0-9\,\ \-]+"));
        episodesRegExp.indexIn(topicsList.at(currentTopic).fullTitle);
        topicsList[currentTopic].episodes = episodesRegExp.cap(0);
        topicsList[currentTopic].episodes.remove(QRegExp(QString::fromUtf8("Сери[и]*[я]*[:]* ")));
    }
    // Try to get total number of episodes if none found yet
    if (!topicsList[currentTopic].episodes.isEmpty() && !topicsList[currentTopic].episodes.contains(QString::fromUtf8(" из ")))
    {
        episodesRegExp.setPattern(QString::fromUtf8("Сери[и]*[я]*[:]* [0-9\,\ \-]+\\([0-9]+\\)"));
        episodesRegExp.indexIn(topicsList.at(currentTopic).fullTitle);
        QString episodesTotal = episodesRegExp.cap(0);
        episodesTotal.remove(QRegExp(QString::fromUtf8("Сери[и]*[я]*[:]* [0-9\,\ \-]+")));
        episodesTotal.remove("(");
        episodesTotal.remove(")");
        if (!episodesTotal.isEmpty())
            topicsList[currentTopic].episodes += QString::fromUtf8(" из ") + episodesTotal;
    }

    /* title */
    topicsList[currentTopic].title = fixTitle(topicsList.at(currentTopic).fullTitle);

    /* year */
    startPos = postBody.indexOf(postParser.yearBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.yearBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].year = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].year.isEmpty())
    {
        topicsList[currentTopic].year = fixString(topicsList[currentTopic].year);
    }

    /* country */
    startPos = postBody.indexOf(postParser.countryBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.countryBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].country = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].country.isEmpty())
    {
        topicsList[currentTopic].country = fixString(topicsList[currentTopic].country);
    }

    /* genre */
    startPos = postBody.indexOf(postParser.genreBegin);
    if (startPos > 0)
    {
       startPos = startPos + postParser.genreBegin.length();
       endPos = postBody.indexOf(postParser.sectionEnd, startPos);
       topicsList[currentTopic].genre = postBody.mid(startPos, endPos - startPos);
   }
    if (!topicsList[currentTopic].genre.isEmpty())
    {
        topicsList[currentTopic].genre = fixString(topicsList[currentTopic].genre).toLower();
    }

    /* duration */
    startPos = postBody.indexOf(postParser.durationBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.durationBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].duration = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].duration.isEmpty())
    {
        topicsList[currentTopic].duration = fixString(topicsList[currentTopic].duration);
    }

    /* translation */
    startPos = postBody.indexOf(postParser.translationBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.translationBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].translation = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].translation.isEmpty())
    {
        topicsList[currentTopic].translation = fixString(topicsList[currentTopic].translation);
    }

    /* director */
    startPos = postBody.indexOf(postParser.directorBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.directorBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].director = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].director.isEmpty())
    {
        topicsList[currentTopic].director = fixString(topicsList[currentTopic].director);
    }

    /* image url */
    startPos = postBody.indexOf(postParser.imageUrlBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.imageUrlBegin.length();
        endPos = postBody.indexOf(postParser.imageUrlEnd, startPos);
        topicsList[currentTopic].imageUrl = postBody.mid(startPos, endPos - startPos);
    }

    /* fallback image url (skip 16+ and 18+ images) */
    if (!topicsList[currentTopic].imageUrl.isEmpty())
    {
        startPos = postBody.indexOf(postParser.imageUrlBegin, endPos + postParser.imageUrlEnd.length());
        if (startPos > 0)
        {
            startPos = startPos + postParser.imageUrlBegin.length();
            endPos = postBody.indexOf(postParser.imageUrlEnd, startPos);
            topicsList[currentTopic].imageUrlFallback = postBody.mid(startPos, endPos - startPos);
        }
    }

    /* quality */
    startPos = postBody.indexOf(postParser.qualityBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.qualityBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].quality = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].quality.isEmpty())
    {
        topicsList[currentTopic].quality = fixString(topicsList[currentTopic].quality);
    }

    /* format */
    startPos = postBody.indexOf(postParser.formatBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.formatBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].format = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].format.isEmpty())
    {
        topicsList[currentTopic].format = fixString(topicsList[currentTopic].format);
    }

    /* video */
    startPos = postBody.indexOf(postParser.videoBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.videoBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].video = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].video.isEmpty())
    {
        topicsList[currentTopic].video = fixString(topicsList[currentTopic].video);
    }

    /* audio 1 */
    startPos = postBody.indexOf(postParser.audioBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.audioBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].audio1 = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].audio1.isEmpty())
    {
        topicsList[currentTopic].audio1 = fixString(topicsList[currentTopic].audio1);
    }

    /* audio 2 */
    if (postBody.count(postParser.audioBegin) > 1)
    {
        startPos = postBody.indexOf(postParser.audioBegin, endPos) + postParser.audioBegin.length();
        endPos = postBody.indexOf(postParser.sectionEnd, startPos);
        topicsList[currentTopic].audio2 = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].audio2.isEmpty())
    {
        topicsList[currentTopic].audio2 = fixString(topicsList[currentTopic].audio2);
    }

    /* audio hack */
    startPos = postBody.indexOf(postParser.audioBegin);
    if (topicsList.at(currentTopic).audio1.isEmpty() && (startPos > 0))
    {
        startPos = startPos + postParser.audioBegin.length();
        endPos = postBody.indexOf(postParser.descriptionEnd, startPos);
        topicsList[currentTopic].audio1 = postBody.mid(startPos, endPos - startPos);
    }

    /* description */
    startPos = postBody.indexOf(postParser.descriptionBegin);
    if (startPos > 0)
    {
        startPos = startPos + postParser.descriptionBegin.length();
        endPos = postBody.indexOf(postParser.descriptionEnd, startPos);
        int altEndPos = postBody.indexOf(postParser.sectionEnd, startPos);
        if (endPos < startPos + 50)
        {
            endPos = postBody.indexOf(postParser.sectionEnd, startPos + 50);
        }
        if (endPos < altEndPos)
        {
            endPos = postBody.indexOf(postParser.descriptionEnd, altEndPos);
        }
        topicsList[currentTopic].description = postBody.mid(startPos, endPos - startPos);
    }
    if (!topicsList[currentTopic].description.isEmpty())
    {
        topicsList[currentTopic].description = fixString(topicsList[currentTopic].description);
    }
}

RutrackerFetcher::~RutrackerFetcher()
{
    delete settings;
}
