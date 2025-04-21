#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include "admcontroller.h"

ADMController::ADMController(QObject *parent)
    : QObject{parent}
{
    m_javaADMController = QAndroidJniObject(
        "org/salieff/SafAdapter",
        "(Landroid/content/Context;J)V",
        QtAndroid::androidContext().object(), reinterpret_cast<jlong>(this)
        );
}

void ADMController::Initialize()
{
    JNINativeMethod methods[] {
        {"DownloadComplete", "(JJII)V", reinterpret_cast<void *>(ADMController::DownloadCompleteDispatcher)},
        {"DownloadProgress", "(JJJJ)V", reinterpret_cast<void *>(ADMController::DownloadProgressDispatcher)}
    };
    QAndroidJniObject javaClass("org/salieff/ADMController");
    QAndroidJniEnvironment env;

    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}

int ADMController::Open(int64_t id)
{
    return QAndroidJniObject::callStaticMethod<jint>(
        "org/salieff/ADMController",
        "Open",
        "(Landroid/content/Context;J)I",
        QtAndroid::androidContext().object(), id
        );
}

void ADMController::Remove(int64_t id)
{
    QAndroidJniObject::callStaticMethod<void>(
        "org/salieff/ADMController",
        "Remove",
        "(Landroid/content/Context;J)V",
        QtAndroid::androidContext().object(), id
        );
}

bool ADMController::sync(QString serverUrl, QString filePath)
{
    return m_javaADMController.callMethod<jboolean>(
               "sync",
               "(Ljava/lang/String;Ljava/lang/String;)Z",
               QAndroidJniObject::fromString(serverUrl).object<jstring>(),
               QAndroidJniObject::fromString(filePath).object<jstring>()
               ) != JNI_FALSE;
}

bool ADMController::download(QString serverUrl, QString filePath, QString title, QString descripton)
{
    return m_javaADMController.callMethod<jboolean>(
               "download",
               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
               QAndroidJniObject::fromString(serverUrl).object<jstring>(),
               QAndroidJniObject::fromString(filePath).object<jstring>(),
               QAndroidJniObject::fromString(title).object<jstring>(),
               QAndroidJniObject::fromString(descripton).object<jstring>()
               ) != JNI_FALSE;
}

int ADMController::open()
{
    return m_javaADMController.callMethod<jint>("open");
}

void ADMController::remove()
{
    m_javaADMController.callMethod<void>("remove");
}

void ADMController::DownloadCompleteDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jint status, jint reason)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    ADMController *object = reinterpret_cast<ADMController *>(cppThisPointer);
    emit object->DownloadComplete(id, status, reason);
}

void ADMController::DownloadProgressDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jlong downloaded, jlong totalSize)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    ADMController *object = reinterpret_cast<ADMController *>(cppThisPointer);
    emit object->DownloadProgress(id, downloaded, totalSize);
}
