#ifndef TTKCIRCULARPROCESSWIDGET_H
#define TTKCIRCULARPROCESSWIDGET_H

/* =================================================
 * This file is part of the TTK Widget Tools project
 * Copyright (C) 2015 - 2021 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ================================================= */

#include <QProgressBar>
#include "ttkglobaldefine.h"

class TTKCircularProgressWidget;

/*!
 * @author Greedysky <greedysky@163.com>
 *   Heikki Johannes <hildenjohannes@gmail.com>
 */
class TTK_MODULE_EXPORT TTKCircularProgressDelegate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal dashOffset WRITE setDashOffset READ dashOffset)
    Q_PROPERTY(qreal dashLength WRITE setDashLength READ dashLength)
    Q_PROPERTY(int angle WRITE setAngle READ angle)
public:
    explicit TTKCircularProgressDelegate(TTKCircularProgressWidget *parent = nullptr);

    void setDashOffset(qreal offset);
    inline qreal dashOffset() const { return m_dashOffset; }

    void setDashLength(qreal length);
    inline qreal dashLength() const { return m_dashLength; }

    void setAngle(int angle);
    inline int angle() const { return m_angle; }

private:
    TTKCircularProgressWidget *m_progress;
    qreal m_dashOffset, m_dashLength;
    int m_angle;

};


/*!
 * @author Greedysky <greedysky@163.com>
 *   Heikki Johannes <hildenjohannes@gmail.com>
 */
class TTK_MODULE_EXPORT TTKCircularProgressWidget : public QProgressBar
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKCircularProgressWidget)
    Q_PROPERTY(qreal lineWidth WRITE setLineWidth READ lineWidth)
    Q_PROPERTY(qreal size WRITE setSize READ size)
    Q_PROPERTY(QColor color WRITE setColor READ color)
public:
    explicit TTKCircularProgressWidget(QWidget *parent = nullptr);
    virtual ~TTKCircularProgressWidget();

    void setLineWidth(qreal width);
    qreal lineWidth() const;

    void setSize(int size);
    int size() const;

    void setColor(const QColor &color);
    QColor color() const;

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    TTKCircularProgressDelegate *m_delegate;
    QColor m_color;
    qreal m_penWidth;
    int m_size;

};

#endif // TTKCIRCULARPROCESSWIDGET_H
