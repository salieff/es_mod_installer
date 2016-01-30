#ifndef ESMODELEMENT_H
#define ESMODELEMENT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

#include "asyncdownloader.h"
#include "asyncunzipper.h"
#include "asyncdeleter.h"

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

    ESModElement(QObject *parent = NULL, State s = Unknown, int p = 100);

    void Download();
    void Abort();
    void Update();
    void Delete();

    void RequestHeaders();

    QJsonObject SerializeToDB();
    void DeserializeFromDB(QJsonObject obj);

    QString title;
    QStringList langs;
    QString status;
    QString uri;
    QString infouri;
    QString path;
    QStringList files;

    State state;
    int progress;
    double size;
    double timestamp;

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
    void downloadOverwriteRequest(QString fname);
    void unzipperOverwriteRequest(QString fname);

signals:
    void stateChanged();
    void removeMe();
    void saveMe();

// private signals:
    void abortProcessing();

private:
    void blockGui(GuiBlockReason b);
    void changeState(State s);

    AsyncDownloader m_asyncDownloader;
    AsyncUnzipper m_asyncUnzipper;
    AsyncDeleter m_asyncDeleter;
};

#endif // ESMODELEMENT_H
