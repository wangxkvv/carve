/*
    Copyright 2008 Jeff Schiller

    This file is part of Carve.

    Carve is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Carve is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Carve.  If not, see http://www.gnu.org/licenses/ .

*/
#ifndef CARVESVGELEMENT_H
#define CARVESVGELEMENT_H

#include "carvesvgnode.h"

class CarveSVGWindow;
class CarveSVGNode;
class QGraphicsRectItem;

/*
enum PreserveAspectRatioType {
    none = 1,
    xMinYMin = 2,
    xMidYMin = 3,
    xMaxYMin = 4,
    xMinYMid = 5,
    xMidYMid = 6,
    xMaxYMid = 7,
    xMinYMax = 8,
    xMidYMax = 9,
    xMaxYMax = 10,
};
enum MeetOrSliceType {
    meet = 1,
    slice = 2,
};
*/

class CarveSVGElement : public CarveSVGNode
{
public:
    CarveSVGElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent = 0);
    virtual ~CarveSVGElement();

    void update(int viewWidth, int viewHeight);

private:
    QRectF viewBox_;
//    PreserveAspectRatioType preserveAspectRatio_;
//    MeetOrSliceType meetOrSlice_;
    Qt::AspectRatioMode arm_;
    qreal width_;
    qreal height_;
    QGraphicsRectItem* canvas_;
    QGraphicsRectItem* viewboxCanvas_;
};

#endif // CARVESVGELEMENT_H
