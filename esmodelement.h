#ifndef ESMODELEMENT_H
#define ESMODELEMENT_H

#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QJsonObject>

#include "asyncfilewriter.h"
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

    ESModElement(QNetworkAccessManager *mgr, QObject *parent = 0);
    ESModElement(State s, int p, QNetworkAccessManager *mgr, QObject *parent = 0);
    virtual ~ESModElement();

    QString StateName() const;

    void Download();
    void Abort();
    void Retry();
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
    void headerReceived();
    void fileDownloaded();
    void fileClosed();
    void fileError(QString strErr);
    void zipListUnpacked();
    void downloadError(QNetworkReply::NetworkError err);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void unpackError(QString strErr);
    void unpackProgress(int p);
    void readData();

signals:
    void stateChanged();
    void stopDownload();

private:
    QNetworkAccessManager *m_NetMgr;
    int m_currDownloadIndex;
    AsyncFileWriter m_currDownloadFile;
    AsyncUnzipper m_asyncUnzipper;
};

#endif // ESMODELEMENT_H
