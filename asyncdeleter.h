#ifndef ASYNCDELETER_H
#define ASYNCDELETER_H

#include <QThread>

#include "modpaths.h"

class AsyncDeleter : public QThread
{
    Q_OBJECT
public:
    explicit AsyncDeleter(QObject *parent = NULL, QString stopFolder = ANDROID_ES_MODS_FOLDER);
    bool deleteFiles(QStringList flist);
    void setStopFolder(QString stopFolder);

protected:
    virtual void run();

private:
    QStringList m_localFiles;
    QString m_parentStopFolder;
};

#endif // ASYNCDELETER_H
