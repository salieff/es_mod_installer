#ifndef ADMCONTROLLER_H
#define ADMCONTROLLER_H

#include <QAndroidJniObject>
#include <QObject>


class ADMController : public QObject
{
    Q_OBJECT
public:
    explicit ADMController(QObject *parent = nullptr);

    static void Initialize();

    static int Open(int64_t id);
    static void Remove(int64_t id);

signals:
    void DownloadComplete(int64_t id, int status, int reason);
    void DownloadProgress(int64_t id, int64_t downloaded, int64_t totalSize);

public slots:
    bool sync(QString serverUrl, QString filePath);
    bool download(QString serverUrl, QString filePath, QString title, QString descripton);
    int open();
    void remove();

private:
    static void DownloadCompleteDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jint status, jint reason);
    static void DownloadProgressDispatcher(JNIEnv *env, jobject thiz, jlong cppThisPointer, jlong id, jlong downloaded, jlong totalSize);

    QAndroidJniObject m_javaADMController;
};

#endif // ADMCONTROLLER_H
