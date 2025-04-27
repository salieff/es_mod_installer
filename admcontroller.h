#ifndef ADMCONTROLLER_H
#define ADMCONTROLLER_H

#include <QJniObject>
#include <QObject>


class ADMController : public QObject
{
    Q_OBJECT
public:
    enum Status {
        STATUS_UNDEFINED = 0,
        STATUS_PENDING = 1,
        STATUS_RUNNING = 2,
        STATUS_PAUSED = 4,
        STATUS_SUCCESSFUL = 8,
        STATUS_FAILED = 16
    };
    Q_ENUM(Status)

    enum Reason {
        REASON_UNDEFINED = 0,
        PAUSED_WAITING_TO_RETRY = 1,
        PAUSED_WAITING_FOR_NETWORK = 2,
        PAUSED_QUEUED_FOR_WIFI = 3,
        PAUSED_UNKNOWN = 4,
        ERROR_UNKNOWN = 1000,
        ERROR_FILE_ERROR = 1001,
        ERROR_UNHANDLED_HTTP_CODE = 1002,
        ERROR_HTTP_DATA_ERROR = 1004,
        ERROR_TOO_MANY_REDIRECTS = 1005,
        ERROR_INSUFFICIENT_SPACE = 1006,
        ERROR_DEVICE_NOT_FOUND = 1007,
        ERROR_CANNOT_RESUME = 1008,
        ERROR_FILE_ALREADY_EXISTS = 1009
    };
    Q_ENUM(Reason)

    explicit ADMController(QObject *parent = nullptr);

    static void Initialize();

    static int Open(qint64 id);
    static void Remove(qint64 id);

    static QString StatusString(Status status);
    static QString ReasonString(Reason reason);

    constexpr static const char *URIScheme = "AndroidDownloadManager://";

signals:
    void DownloadComplete(qint64 id, ADMController::Status status, ADMController::Reason reason);
    void DownloadProgress(qint64 id, qint64 downloaded, qint64 totalSize);

public slots:
    bool sync(QString serverUrl, QString filePath);
    bool download(QString serverUrl, QString filePath, QString title, QString descripton);
    int open();
    void remove();

private:
    static void DownloadCompleteDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jint status, jint reason);
    static void DownloadProgressDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jlong downloaded, jlong totalSize);

    QJniObject m_javaADMController;
};

#endif // ADMCONTROLLER_H
