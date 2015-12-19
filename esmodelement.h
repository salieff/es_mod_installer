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

    ESModElement(QObject *parent = NULL, State s = Unknown, int p = 100);

    QString StateName() const;

    void Download();
    void Abort();
    void Update();
    void Delete();

    void RequestHeaders();

    QJsonObject SerializeToDB();
    void DeserializeFromDB(QJsonObject obj);

    QString title;
    QString uri;
    QString path;
    QStringList files;

    State state;
    int progress;
    double size;
    double timestamp;

    int guiblocked;

    QStringList m_localFiles;
    double m_localSize;
    double m_localTimestamp;

private slots:
    void headersReceived();
    void filesDownloaded();
    void downloadProgress(int p);
    void zipListUnpacked();
    void unpackProgress(int p);
    void filesDeleted();

signals:
    void stateChanged();
    void removeMe();
    void saveMe();

// private signals:
    void abortProcessing();

private:
    void blockGui(int b = 1);
    void changeState(State s);

    AsyncDownloader m_asyncDownloader;
    AsyncUnzipper m_asyncUnzipper;
    AsyncDeleter m_asyncDeleter;
};

#endif // ESMODELEMENT_H
