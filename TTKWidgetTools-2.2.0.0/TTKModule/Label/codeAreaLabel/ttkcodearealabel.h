#ifndef TTKCODEAREALABEL_H
#define TTKCODEAREALABEL_H

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

#include <QLabel>
#include <QPainterPath>
#include "ttkglobaldefine.h"

/*!
* @author Greedysky <greedysky@163.com>
*/
class TTK_MODULE_EXPORT TTKCodeAreaLabel : public QLabel
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKCodeAreaLabel)
public:
    explicit TTKCodeAreaLabel(QWidget *parent = nullptr);

    void renderPicture();
    inline QString getCheckCode() const { return m_sCode; }

    void setCodeCount(int nCount);
    inline int getCodeCount() const { return m_nCodeCount; }

    void setNoisyPointCount(int nCount);
    inline int getNoisyPointCount() const { return m_nNoisyPointCount; }

    virtual QSize sizeHint() const override;

protected:
    void drawOutline(QPainter &painter);
    void drawNoisyPoint(QPainter &painter);
    void drawConversion(QPainter &painter);
    virtual void paintEvent(QPaintEvent *event) override;

    int m_nNoisyPointCount, m_nConverseRotate;
    int m_nConverseScale, m_nCodeCount;
    QString m_sCode;
    QStringList m_slCodeRange;
    QList<QPainterPath> m_lCodePic;
    QList<Qt::GlobalColor> m_lCodeColor;

};

#endif // TTKCODEAREALABEL_H
