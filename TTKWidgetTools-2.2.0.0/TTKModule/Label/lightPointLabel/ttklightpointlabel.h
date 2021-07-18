#ifndef TTKLIGHTPOINTLABEL_H
#define TTKLIGHTPOINTLABEL_H

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

#include <QWidget>
#include "ttkglobaldefine.h"

/*!
* @author Greedysky <greedysky@163.com>
*         feiyangqingyun <feiyangqingyun@163.com>
*/
class TTK_MODULE_EXPORT TTKLightPointLabel : public QWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKLightPointLabel)
public:
    explicit TTKLightPointLabel(QWidget *parent = nullptr);
    ~TTKLightPointLabel();

    void setStep(int step);
    void setInterval(int interval);
    void setBgColor(const QColor &bgColor);

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    void drawBg(QPainter *painter);

private:
    int m_step;
    int m_interval;
    QColor m_bgColor;

    int m_offset;
    bool m_add;
    QTimer *m_timer;

};

#endif // TTKLIGHTPOINTLABEL_H
