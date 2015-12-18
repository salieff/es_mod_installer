#ifndef ESMODELEMENT_H
#define ESMODELEMENT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

#include "asyncdownloader.h"
#include "asyncunzipper.h"

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

    QStringList m_localFiles;
    double m_localSize;
    double m_localTimestamp;

public slots:
    void zipListUnpacked();
    void unpackProgress(int p);

private slots:
    void headersReceived();
    void filesDownloaded();
    void downloadProgress(int p);

signals:
    void stateChanged();

// private signals:
    void abortProcessing();

private:
    AsyncDownloader m_asyncDownloader;
    AsyncUnzipper m_asyncUnzipper;
};

#endif // ESMODELEMENT_H
