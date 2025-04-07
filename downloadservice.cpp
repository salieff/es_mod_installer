#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QMessageBox>
#include <android/log.h>

#include "downloadservice.h"


long DownloadService::StartDownload(const QString &url)
{
    return QAndroidJniObject::callStaticMethod<jlong>("org/salieff/DownloadService",
                                                      "StartDownload",
                                                      "(Landroid/content/Context;Ljava/lang/String;)J",
                                                      QtAndroid::androidContext().object(),
                                                      QAndroidJniObject::fromString(url).object<jstring>());
}

void DownloadService::DownloadComplete(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    // QMessageBox::information(NULL, "Download complete", QString("Id = %1").arg(id));
    __android_log_write(ANDROID_LOG_DEBUG, "DownloadService", QString("Download complete Id = %1").arg(id).toLocal8Bit().constData());
}

void DownloadService::RegisterJNINativeMethods()
{
    JNINativeMethod methods[] {{"DownloadComplete", "(J)V", reinterpret_cast<void *>(DownloadService::DownloadComplete)}};
    QAndroidJniObject javaClass("org/salieff/DownloadService");

    QAndroidJniEnvironment env;

    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}

void DownloadService::RegisterReceiver()
{
    QAndroidJniObject::callStaticMethod<void>("org/salieff/DownloadService",
                                              "RegisterReceiver",
                                              "(Landroid/content/Context;)V",
                                              QtAndroid::androidContext().object());
}
