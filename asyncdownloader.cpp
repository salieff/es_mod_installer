#include <QAndroidJniObject>
#include <QtAndroid>

#include "asyncdownloader.h"


QNetworkAccessManager * AsyncDownloader::m_networkManager = NULL;
QString AsyncDownloader::m_myUDID;


void AsyncDownloader::createNetworkManager(QObject *parent)
{
    if (m_networkManager != NULL)
        return;

    m_networkManager = new QNetworkAccessManager(parent);
}

QString AsyncDownloader::getDeviceUDID()
{
    if (m_myUDID.isEmpty())
    {
        QAndroidJniObject myID = QAndroidJniObject::fromString("android_id");
        QAndroidJniObject contentR = QtAndroid::androidContext().callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");
        QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod("android/provider/Settings$Secure", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;",contentR.object(), myID.object<jstring>());

        m_myUDID = result.toString();
    }

    return m_myUDID;
}

QNetworkReply * AsyncDownloader::get(QString url)
{
    return get(QUrl(url));
}

QNetworkReply * AsyncDownloader::get(QString baseUrl, QString fileUrl)
{
    return get(QUrl(baseUrl).resolved(QUrl(fileUrl)));
}

QNetworkReply * AsyncDownloader::get(QUrl url)
{
    QNetworkRequest r;
    r.setUrl(url);
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    return m_networkManager->get(r);
}

QNetworkReply * AsyncDownloader::head(QString url)
{
    return head(QUrl(url));
}

QNetworkReply * AsyncDownloader::head(QString baseUrl, QString fileUrl)
{
    return head(QUrl(baseUrl).resolved(QUrl(fileUrl)));
}

QNetworkReply * AsyncDownloader::head(QUrl url)
{
    QNetworkRequest r;
    r.setUrl(url);
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    return m_networkManager->head(r);
}
