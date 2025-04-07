#ifndef DOWNLOADSERVICE_H
#define DOWNLOADSERVICE_H

#include <QString>
#include <QAndroidJniObject>


class DownloadService
{
public:
    static long StartDownload(const QString &url);
    static void DownloadComplete(JNIEnv *env, jobject thiz, jlong id);
    static void RegisterJNINativeMethods();
    static void RegisterReceiver();
};

#endif // DOWNLOADSERVICE_H
