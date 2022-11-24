#ifndef ASYNCDELETER_H
#define ASYNCDELETER_H

#include <QThread>


class AsyncDeleter : public QThread
{
    Q_OBJECT
public:
    explicit AsyncDeleter(QObject *parent = NULL);
    bool deleteFiles(QStringList flist);

protected:
    virtual void run();

private:
    QStringList m_localFiles;
};

#endif // ASYNCDELETER_H
