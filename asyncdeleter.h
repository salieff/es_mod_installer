#ifndef ASYNCDELETER_H
#define ASYNCDELETER_H

#include <QThread>

#include "modpaths.h"

class AsyncDeleter : public QThread
{
    Q_OBJECT
public:
    explicit AsyncDeleter(QObject *parent = NULL, QString stopFolder = QString("%1/%2").arg(ANDROID_ES_MODS_EXTERNAL_STORAGE, ANDROID_ES_MODS_FOLDER));
    bool deleteFiles(QStringList flist);
    void setStopFolder(QString stopFolder);

protected:
    virtual void run();

private:
    void recurseDeleteEmptyDirs(const QString &childPath);

    QStringList m_localFiles;
    QString m_parentStopFolder;
};

#endif // ASYNCDELETER_H
