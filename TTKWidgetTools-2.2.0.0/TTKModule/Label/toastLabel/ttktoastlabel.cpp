#include "ttktoastlabel.h"

#include <QPainter>
#include <QPropertyAnimation>

TTKToastLabel::TTKToastLabel(QWidget *parent)
    : QLabel(parent)
{
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

#ifndef TTK_BUILD_EXAMPLE
    setAttribute(Qt::WA_QuitOnClose);
    setAttribute(Qt::WA_DeleteOnClose);
#endif

    m_font = font();
    connect(&m_timer, SIGNAL(timeout()), SLOT(updateRender()));
    m_timer.setInterval(1500);
    m_timer.start();
}

TTKToastLabel::TTKToastLabel(const QString &text, QWidget *parent)
    : TTKToastLabel(parent)
{
    setText(text);
}

TTKToastLabel::~TTKToastLabel()
{
    m_timer.stop();
}

void TTKToastLabel::setFontMargin(int height, int width)
{
    m_margin.setX(height);
    m_margin.setY(width);
    update();
}

void TTKToastLabel::setTimerInterval(int msecond)
{
    m_timer.stop();
    m_timer.setInterval(msecond);
    m_timer.start();
}

int TTKToastLabel::getTimerInterval() const
{
    return m_timer.interval();
}

void TTKToastLabel::setFontSize(int size)
{
    m_font.setPointSize(size);
    setFont(m_font);
    update();
}

int TTKToastLabel::getFontSize() const
{
    return m_font.pointSize();
}

void TTKToastLabel::setBold(bool bold)
{
    m_font.setBold(bold);
    setFont(m_font);
    update();
}

bool TTKToastLabel::bold() const
{
    return m_font.bold();
}

void TTKToastLabel::popup(QWidget *parent)
{
    QPoint globalPoint = parent->mapToGlobal(QPoint(0, 0));
    move( globalPoint.x() + (parent->width() - width())/2,
          globalPoint.y() + (parent->height() - height())/2);
    raise();
    show();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setDuration(500);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
}

void TTKToastLabel::setText(const QString &text)
{
    QFontMetrics metrics = QFontMetrics(m_font);
#if TTK_QT_VERSION_CHECK(5,13,0)
    const int w = metrics.horizontalAdvance(text);
#else
    const int w = metrics.width(text);
#endif
    setFixedSize(w + m_margin.x(), metrics.height() + m_margin.y());
    QLabel::setText(text);
}

void TTKToastLabel::updateRender()
{
    m_timer.stop();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setDuration(1000);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
    connect(animation, SIGNAL(finished()), SLOT(close()));
}

void TTKToastLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 175));
#if TTK_QT_VERSION_CHECK(5,13,0)
    painter.drawRoundedRect(rect(), 6, 6);
#else
    painter.drawRoundRect(rect(), 6, 6);
#endif

    painter.setPen(QColor(255, 255, 255));
    painter.drawText(rect(), Qt::AlignCenter, text());
    painter.end();
}
