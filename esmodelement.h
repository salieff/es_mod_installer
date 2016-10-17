#ifndef ESMODELEMENT_H
#define ESMODELEMENT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

#include "asyncdownloader.h"
#include "asyncunzipper.h"
#include "asyncdeleter.h"

#define LIKES_CGI_URL "http://es.191.ru/cgi-bin/ratingsystem/rating_web.py"
#define STATS_CGI_URL LIKES_CGI_URL

class ESModElement : public QObject
{
    Q_OBJECT
public:
    enum State {
        Unknown,
        Available,
        Downloading,
        Unpacking,
        Failed,
        InstalledAvailable,
        InstalledHasUpdate,
        Installed
    };
    Q_ENUMS(State)

    enum GuiBlockReason {
        NoBlock,
        ByUnknown,
        ByDownload,
        ByUnpack,
        ByAbort,
        ByRetry,
        ByUpdate,
        ByDelete
    };
    Q_ENUMS(GuiBlockReason)

    enum LikeType {
        LikeMarkNotFound,
        LikeMark,
        DislikeMark
    };
    Q_ENUMS(LikeType)

    ESModElement(QString au = QString(), QString ap = QString(), QObject *parent = NULL, State st = Unknown, int pr = 100);

    void Download();
    void Abort();
    void Update();
    void Delete();
    void SendLike(LikeType l);

    void RequestHeaders();
    QString errorString();

    QJsonObject SerializeToDB();
    void DeserializeFromDB(const QJsonObject &obj);
    bool DeserializeFromNetwork(const QJsonObject &obj);
    void DeserializeFromAllLikesList(const QJsonObject &obj);
    void DeserializeFromAllStatisticsList(const QJsonObject &obj);
    void SetInstallPath(QString p);

    void TryToPickupFrom(QList<ESModElement *> &list);

    int id;
    QString title;
    QStringList langs;
    QString status;
    QString infouri;
    QStringList files;

    State state;
    int progress;
    double size;
    double timestamp;

    // Likes
    LikeType mylikemark;
    int likemarkscount;
    int dislikemarkscount;

    // Installations statistics
    int insttotal;
    int instactive;
    int insttotalmonth;
    int instactivemonth;
    int insttotalweek;
    int instactiveweek;
    int lifetimeavg;
    int lifetimemax;

    GuiBlockReason guiblocked;

    QStringList m_localFiles;
    double m_localSize;
    double m_localTimestamp;

    int m_modelIndex;
    std::vector<int> m_keywordFilterCounter;

private slots:
    void headersReceived();
    void filesDownloaded();
    void downloadProgress(int p);
    void zipListUnpacked();
    void unpackProgress(int p);
    void filesDeleted();
    void unzipperOverwriteRequest(QString fname);
    void allLikesReceived();
    void myLikeReceived();
    void myLikePosted();
    void subDownload();

signals:
    void stateChanged();
    void removeMe();
    void saveMe();

// private signals:
    void abortProcessing();

private:
    void blockGui(GuiBlockReason b);
    void changeState(State s, int resumedProgress = -1);

    void sendLikesRequests();
    void sendStatistics(bool inst = true);

    bool idEquals(ESModElement *el);

    AsyncDownloader m_asyncDownloader;
    AsyncUnzipper m_asyncUnzipper;
    AsyncDeleter m_asyncDeleter;

    QString m_uri;
    QString m_path;

    int m_failedDownloadsCount;
};

#endif // ESMODELEMENT_H
