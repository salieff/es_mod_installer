// #include <iostream>
#include "esswipearea.h"
#include <QDebug>

ESSwipeArea::ESSwipeArea(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_pixelage(0),
      m_speed(0),
      m_state(WaitingPress),
      m_timer(this)
{
    setFiltersChildMouseEvents(true);
    m_timer.setInterval(100);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(processCollectedData()));
}

qint32 ESSwipeArea::pixelage() const
{
    return m_pixelage;
}

void ESSwipeArea::setPixelage(qint32 p)
{
    m_pixelage = p;
}

qreal ESSwipeArea::speed() const
{
    return m_speed;
}

void ESSwipeArea::setSpeed(qreal s)
{
    m_speed = s;
}

void ESSwipeArea::paint(QPainter *painter)
{
}

bool ESSwipeArea::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    qDebug() << "[FILTER] " << event->type();

    switch (event->type())
    {
    case QEvent::MouseButtonPress :
        if (m_state == InSwipe)
            return true;

        if (m_state == PassThrough)
            return false;

        if (m_state != WaitingPress)
            return false;

        m_suspendedEvents.clear();
        m_suspendedEvents.push_back(std::pair<QQuickItem *, QMouseEvent>(item, *(reinterpret_cast<QMouseEvent *>(event))));
        m_state = Collect;
        m_startTimer.start();
        m_timer.start();
        return true;

    case QEvent::MouseMove :
        if (m_state == InSwipe)
            return true;

        if (m_state == PassThrough)
            return false;

        if (m_state != Collect)
            return false;

        m_suspendedEvents.push_back(std::pair<QQuickItem *, QMouseEvent>(item, *(reinterpret_cast<QMouseEvent *>(event))));
        return true;

    case QEvent::MouseButtonRelease :
        if (m_state == InSwipe)
        {
            m_state = WaitingPress;
            return true;
        }

        if (m_state == PassThrough)
        {
            m_state = WaitingPress;
            return false;
        }

        if (m_state != Collect)
        {
            m_state = WaitingPress;
            return false;
        }

        stopProcessing(true);
        m_state = WaitingPress;
        return false;

    default :
        break;
    }

    return false;
}

void ESSwipeArea::processCollectedData()
{
    if (m_suspendedEvents.size() < 2)
    {
        stopProcessing(true);
        return;
    }

    for (size_t i = 2; i < m_suspendedEvents.size(); ++i)
    {
        if (m_suspendedEvents[1].second.pos().x() > m_suspendedEvents.front().second.pos().x() && \
                m_suspendedEvents[i].second.pos().x() < m_suspendedEvents[i - 1].second.pos().x())
        {
            qDebug() << "Revers right -> left";
            stopProcessing(true);
            return;
        }

        if (m_suspendedEvents[1].second.pos().x() < m_suspendedEvents.front().second.pos().x() && \
                m_suspendedEvents[i].second.pos().x() > m_suspendedEvents[i - 1].second.pos().x())
        {
            qDebug() << "Revers left -> right";
            stopProcessing(true);
            return;
        }
    }

    int minY = m_suspendedEvents.front().second.pos().y();
    int maxY = m_suspendedEvents.front().second.pos().y();
    for (size_t i = 1; i < m_suspendedEvents.size(); ++i)
    {
        if (m_suspendedEvents[i].second.pos().y() < minY)
            minY = m_suspendedEvents[i].second.pos().y();

        if (m_suspendedEvents[i].second.pos().y() > maxY)
            maxY = m_suspendedEvents[i].second.pos().y();
    }

    int deltaX = m_suspendedEvents.back().second.pos().x() - m_suspendedEvents.front().second.pos().x();
    int deltaY = maxY - minY;
    if (abs(deltaX) * 2 < 3 * abs(deltaY))
    {
        qDebug() << "Global angle > 30g";
        stopProcessing(true);
        return;
    }

    if (abs(deltaX) * 1000.0 < m_speed * m_startTimer.elapsed())
    {
        qDebug() << "Global speed " << abs(deltaX) * 1000.0 / m_startTimer.elapsed() << " < " << m_speed;
        stopProcessing(true);
        return;
    }

    if (abs(deltaX) >= m_pixelage)
    {
        stopProcessing(false);

        if (deltaX < 0)
        {
            qDebug() << "Swipe LEFT!";
            emit swipeLeft();
        }
        else
        {
            qDebug() << "Swipe RIGHT!";
            emit swipeRight();
        }

        return;
    }
}

void ESSwipeArea::stopProcessing(bool revert)
{
    m_timer.stop();

    if (revert)
    {
        for (size_t i = 0; i < m_suspendedEvents.size(); ++i)
            QCoreApplication::sendEvent(m_suspendedEvents[i].first, &m_suspendedEvents[i].second);

        m_state = PassThrough;
    }
    else
    {
        m_state = InSwipe;
    }

    m_suspendedEvents.clear();
}
