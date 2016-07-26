#include <iostream>
#include "esswipearea.h"

ESSwipeArea::ESSwipeArea(QQuickItem *parent) : QQuickItem(parent), m_state(WaitingPress), m_timer(this)
{
    setFiltersChildMouseEvents(true);
    m_timer.setInterval(1000);
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(breakAnalyze()));
}

bool ESSwipeArea::childMouseEventFilter(QQuickItem *, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress :
        if (m_state != WaitingPress)
            return false;

        startAnalyze(event);
        return true;

    case QEvent::MouseMove :
        return analyze(event);

    case QEvent::MouseButtonRelease :
        return analyze(event, true);

    default :
        break;
    }

    return false;
}

void ESSwipeArea::breakAnalyze()
{
    m_state = PassThrough;
    for (size_t i = 0; i < m_suspendedEvents.size(); ++i)
    {
        // Send events to children
    }
}

void ESSwipeArea::startAnalyze(QEvent *event)
{
    m_suspendedEvents.clear();

    QMouseEvent me(*(reinterpret_cast<QMouseEvent *>(event)));
    m_startTimer.start();
    m_startPoint = me.pos();
    m_suspendedEvents.push_back(me);
    m_state = Analyze;
    m_timer.start();
}

bool ESSwipeArea::analyze(QEvent *event, bool release)
{
    if (m_state == InSwipe)
    {
        if (release)
            m_state = WaitingPress;

        return true;
    }

    if (m_state == PassThrough)
    {
        if (release)
            m_state = WaitingPress;

        return false;
    }

    if (m_state != Analyze)
    {
        if (release)
            m_state = WaitingPress;

        return false;
    }

    m_timer.stop();
    QMouseEvent me(*(reinterpret_cast<QMouseEvent *>(event)));

    if (0)
    {
        m_suspendedEvents.clear();

        if (release)
            m_state = WaitingPress;
        else
            m_state = InSwipe;

        emit swipeLeft();
        return true;
    }
    else if (0)
    {
        m_suspendedEvents.clear();

        if (release)
            m_state = WaitingPress;
        else
            m_state = InSwipe;

        emit swipeRight();
        return true;
    }

    if (0 || release)
    {
        breakAnalyze();

        if (release)
            m_state = WaitingPress;

        return false;
    }

    m_suspendedEvents.push_back(me);
    m_timer.start();
    return true;
}
