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
#include "carvelineelement.h"
#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsLineItem>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>

#include <iostream>
using std::cout;
using std::endl;

CarveLineElement::CarveLineElement(const QDomElement& node, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
    CarveSVGNode(node, row, window, svgLine, parent)
{
    bool bOk = false;

    qreal x1 = getFloatTrait(node, "x1",&bOk);
    if(!bOk) {
	x1 = 0.0;
    }

    qreal y1 = getFloatTrait(node, "y1",&bOk);
    if(!bOk) {
	y1 = 0.0;
    }

    qreal x2 = getFloatTrait(node, "x2",&bOk);
    if(!bOk) {
	x2 = 0.0;
    }

    qreal y2 = getFloatTrait(node, "y2",&bOk);
    if(!bOk) {
	y2 = 0.0;
    }

    CarveGraphicsLineItem* item = new CarveGraphicsLineItem(window, x1, y1, x2, y2);
    finishDecorating(item);
    item->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
    this->getStrokeLineCap();
    this->getStrokeLineJoin();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    item->setPen(this->getStroke(&bOk));
}

CarveLineElement::~CarveLineElement() {
}
