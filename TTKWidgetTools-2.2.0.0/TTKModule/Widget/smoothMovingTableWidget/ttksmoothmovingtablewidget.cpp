#include "ttksmoothmovingtablewidget.h"

#include <QTimer>
#include <QScrollBar>
#include <QWheelEvent>
#include <QPropertyAnimation>

TTKSmoothMovingTableWidget::TTKSmoothMovingTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    m_deltaValue = 0;
    m_previousValue = 0;
    m_isFirstInit = true;
    m_slowAnimation = nullptr;
    m_scrollBar = nullptr;
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(100);

    connect(m_animationTimer, SIGNAL(timeout()), SLOT(updateRender()));
}

TTKSmoothMovingTableWidget::~TTKSmoothMovingTableWidget()
{
    m_animationTimer->stop();
    delete m_animationTimer;
    delete m_slowAnimation;
}

void TTKSmoothMovingTableWidget::setMovedScrollBar(QScrollBar *bar)
{
    m_scrollBar = bar;
    delete m_slowAnimation;
    m_slowAnimation = new QPropertyAnimation(m_scrollBar, "value", this);
    m_slowAnimation->setDuration(1000);
    connect(m_scrollBar, SIGNAL(valueChanged(int)), SLOT(valueChanged(int)));
}

QSize TTKSmoothMovingTableWidget::sizeHint() const
{
    return QSize(180, 180);
}

void TTKSmoothMovingTableWidget::updateRender()
{
    m_isFirstInit = true;
    m_animationTimer->stop();

    float delta = (rowCount() > 0) ? (height()*1.0/rowCount()) : 0;
    m_deltaValue = (m_deltaValue/480.0)*(m_deltaValue < 0 ? m_deltaValue + 120 : -m_deltaValue + 120);

    m_slowAnimation->setStartValue(m_previousValue);
    m_slowAnimation->setEndValue(m_scrollBar->value() + m_deltaValue*delta/30);
    m_slowAnimation->start();
}

void TTKSmoothMovingTableWidget::valueChanged(int value)
{
    m_previousValue = value;
}

void TTKSmoothMovingTableWidget::wheelEvent(QWheelEvent *event)
{
    QTableWidget::wheelEvent(event);

    if(!m_slowAnimation)
    {
        return;
    }

    m_animationTimer->stop();
    m_slowAnimation->stop();
    if(m_isFirstInit)
    {
        m_deltaValue = 0;
        m_previousValue = m_scrollBar->value();
        m_isFirstInit = false;
    }
#if TTK_QT_VERSION_CHECK(5,15,0)
    m_deltaValue += event->angleDelta().y();
#else
    m_deltaValue += event->delta();
#endif
    m_animationTimer->start();
}
