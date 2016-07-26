#ifndef ESSWIPEAREA_H
#define ESSWIPEAREA_H

#include <QtQuick/QQuickItem>
#include <QElapsedTimer>
#include <QTimer>

#include <vector>

class ESSwipeArea : public QQuickItem
{
    Q_OBJECT

public:
    enum State {
        WaitingPress,
        Analyze,
        InSwipe,
        PassThrough
    };

    ESSwipeArea(QQuickItem *parent = 0);
    bool childMouseEventFilter(QQuickItem *, QEvent *event);

public slots:
    void breakAnalyze();

signals:
    void swipeLeft();
    void swipeRight();

private:
    void startAnalyze(QEvent *event);
    bool analyze(QEvent *event, bool release = false);

    State m_state;
    QTimer m_timer;
    QElapsedTimer m_startTimer;
    QPoint m_startPoint;
    QPoint m_lastPoint;
    std::vector<QMouseEvent> m_suspendedEvents;
};

#endif // ESSWIPEAREA_H
