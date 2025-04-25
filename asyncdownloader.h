#ifndef ASYNCDOWNLOADER_H
#define ASYNCDOWNLOADER_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QNetworkAccessManager>


class AsyncDownloader : public QObject
{
    Q_OBJECT
public:
    static void createNetworkManager(QObject *parent = NULL);
    static QString getDeviceUDID();
    static QNetworkReply * get(QString url);
    static QNetworkReply * get(QString baseUrl, QString fileUrl);
    static QNetworkReply * get(QUrl url);
    static QNetworkReply * head(QString url);
    static QNetworkReply * head(QString baseUrl, QString fileUrl);
    static QNetworkReply * head(QUrl url);

private:
    static QNetworkAccessManager *m_networkManager;
    static QString m_myUDID;
};

#endif // ASYNCDOWNLOADER_H
