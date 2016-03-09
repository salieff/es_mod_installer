#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QJsonArray>

class StatisticsManager : public QObject
{
    Q_OBJECT
public:
    static StatisticsManager *getInstance(QObject *parent = 0);
    static void destroyInstance();

    void addRequest(const QString &reqStr);
    void deserializeFromJSON(const QJsonArray &reqArr);
    QJsonArray serializeToJSON();

signals:
    void saveMe();

private slots:
    void requestFinished();

private:
    explicit StatisticsManager(QObject *parent = 0);
    static StatisticsManager *m_internalInstance;

    QStringList m_requestQueue;
};

#endif // STATISTICSMANAGER_H
