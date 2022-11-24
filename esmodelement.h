#ifndef ESMODELEMENT_H
#define ESMODELEMENT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

#include "asyncdownloader.h"
#include "asyncunzipper.h"
#include "asyncdeleter.h"

#define LIKES_CGI_URL "http://879167-dreamtale3.tmweb.ru/cgi-bin/ratingsystem/rating_web.py"
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
    Q_ENUM(State)

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
    Q_ENUM(GuiBlockReason)

    enum LikeType {
        LikeMarkNotFound,
        LikeMark,
        DislikeMark
    };
    Q_ENUM(LikeType)

    ESModElement(QString url = QString(), QObject *parent = NULL, State state = Unknown, int progress = 100);

    void Download(void);
    void Abort(void);
    void Update(void);
    void Delete(void);
    void SendLike(LikeType l);
    void ToggleFavorite(void);

    void RequestHeaders();
    QString errorString();

    QJsonObject SerializeToDB();
    void DeserializeFromDB(const QJsonObject &obj);
    bool DeserializeFromNetwork(const QJsonObject &obj);
    bool DeserializeFromAllLikesList(const QJsonObject &obj);
    void DeserializeFromAllStatisticsList(const QJsonObject &obj);

    void TryToPickupFrom(QList<ESModElement *> &list);

    void EraseFromLocalFiles(const QString &ext);

    int id = -1;
    QString title = "Test sample mod name";
    QStringList langs = QStringList() << "Ru" << "En" <<"Spa";
    QString status = "окончен";
    QString infouri;
    QStringList files;

    State state;
    int progress;
    double size = 0;
    double timestamp = 0;
    bool favorite = false;

    // Likes
    LikeType mylikemark = LikeMarkNotFound;
    int likemarkscount = -1;
    int dislikemarkscount = -1;

    // Installations statistics
    int insttotal = -1;
    int instactive = -1;
    int insttotalmonth = -1;
    int instactivemonth = -1;
    int insttotalweek = -1;
    int instactiveweek = -1;
    int lifetimeavg = -1;
    int lifetimemax = -1;

    GuiBlockReason guiblocked = ByUnknown;

    QStringList m_localFiles;
    double m_localSize = 0;
    double m_localTimestamp = 0;

    int m_modelIndex = -1;
    std::vector<int> m_keywordFilterCounter;

public slots:
    void headersReceived();

private slots:
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

    QString removeOldLocalFilePrefixes(QString filePath);
    void addLocalFilesWithoutOldPrefixes(const QJsonValue jvr);

    AsyncDownloader m_asyncDownloader;
    AsyncUnzipper m_asyncUnzipper;
    AsyncDeleter m_asyncDeleter;

    QString m_uri;

    int m_failedDownloadsCount = 0;
};

#endif // ESMODELEMENT_H
