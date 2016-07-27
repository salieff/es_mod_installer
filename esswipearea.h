#ifndef ESSWIPEAREA_H
#define ESSWIPEAREA_H

#include <QtQuick/QQuickPaintedItem>
#include <QElapsedTimer>
#include <QTimer>

#include <vector>
#include <map>

class ESSwipeArea : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qint32 pixelage READ pixelage WRITE setPixelage)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed)

public:
    enum State {
        WaitingPress,
        Collect,
        InSwipe,
        PassThrough
    };

    ESSwipeArea(QQuickItem *parent = 0);

    qint32 pixelage() const;
    void setPixelage(qint32 p);

    qreal speed() const;
    void setSpeed(qreal s);

    void paint(QPainter *painter);
    bool childMouseEventFilter(QQuickItem *item, QEvent *event);

public slots:
    void processCollectedData();

signals:
    void swipeLeft();
    void swipeRight();

private:
    void stopProcessing(bool revert);

    qint32 m_pixelage;
    qreal m_speed;

    State m_state;
    QElapsedTimer m_startTimer;
    QTimer m_timer;
    std::vector<std::pair<QQuickItem *, QMouseEvent> > m_suspendedEvents;
};

#endif // ESSWIPEAREA_H
