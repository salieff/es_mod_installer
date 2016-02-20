#ifndef ASYNCJSONWRITER_H
#define ASYNCJSONWRITER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QJsonObject>

class AsyncJsonWriter : public QThread
{
    Q_OBJECT
public:
    static QString configFileName();

    AsyncJsonWriter(QObject * parent = 0);
    virtual ~AsyncJsonWriter();

    void write(QJsonObject *obj);
    void close();

protected:
    virtual void run();

private:
    QJsonObject *m_jsonObject;
    QMutex m_jsonMutex;
    QWaitCondition m_jsonCondition;
    bool m_closeFlag;
};

#endif // ASYNCJSONWRITER_H
