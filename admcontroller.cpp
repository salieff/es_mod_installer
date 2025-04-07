#include <QtAndroid>
#include <QAndroidJniEnvironment>

#include "admcontroller.h"


ADMController::ADMController(QObject *parent)
    : QObject{parent}
{
    m_javaADMController = QAndroidJniObject(
        "org/salieff/ADMController",
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

int ADMController::Open(qint64 id)
{
    return QAndroidJniObject::callStaticMethod<jint>(
        "org/salieff/ADMController",
        "Open",
        "(Landroid/content/Context;J)I",
        QtAndroid::androidContext().object(), id
        );
}

void ADMController::Remove(qint64 id)
{
    QAndroidJniObject::callStaticMethod<void>(
        "org/salieff/ADMController",
        "Remove",
        "(Landroid/content/Context;J)V",
        QtAndroid::androidContext().object(), id
        );
}

QString ADMController::StatusString(Status status)
{
#define CASE_STATUS(arg) case STATUS_##arg: return #arg;
    switch (status) {
        CASE_STATUS(UNDEFINED)
        CASE_STATUS(PENDING)
        CASE_STATUS(RUNNING)
        CASE_STATUS(PAUSED)
        CASE_STATUS(SUCCESSFUL)
        CASE_STATUS(FAILED)

    default:
        break;
    }
#undef CASE_STATUS

    return "Unknown status";
}

QString ADMController::ReasonString(Reason reason)
{
#define CASE_REASON(arg) case arg: return #arg;
    switch (reason) {
        CASE_REASON(REASON_UNDEFINED)
        CASE_REASON(PAUSED_WAITING_TO_RETRY)
        CASE_REASON(PAUSED_WAITING_FOR_NETWORK)
        CASE_REASON(PAUSED_QUEUED_FOR_WIFI)
        CASE_REASON(PAUSED_UNKNOWN)
        CASE_REASON(ERROR_UNKNOWN)
        CASE_REASON(ERROR_FILE_ERROR)
        CASE_REASON(ERROR_UNHANDLED_HTTP_CODE)
        CASE_REASON(ERROR_HTTP_DATA_ERROR)
        CASE_REASON(ERROR_TOO_MANY_REDIRECTS)
        CASE_REASON(ERROR_INSUFFICIENT_SPACE)
        CASE_REASON(ERROR_DEVICE_NOT_FOUND)
        CASE_REASON(ERROR_CANNOT_RESUME)
        CASE_REASON(ERROR_FILE_ALREADY_EXISTS)

    default:
        break;
    }
#undef CASE_REASON

    return "Unknown reason";
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
    emit object->DownloadComplete(id, static_cast<ADMController::Status>(status), static_cast<ADMController::Reason>(reason));
}

void ADMController::DownloadProgressDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jlong downloaded, jlong totalSize)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    ADMController *object = reinterpret_cast<ADMController *>(cppThisPointer);
    emit object->DownloadProgress(id, downloaded, totalSize);
}
