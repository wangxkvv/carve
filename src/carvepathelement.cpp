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
#include "carvepathelement.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QtGlobal>

#include <iostream>
using std::cout;
using std::endl;

CarvePathElement::CarvePathElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgPath, parent)
{
    bool bOk = false;

    QPainterPath path(getPathTrait(element, "d", &bOk));

    if(getTrait(element, "fill-rule", &bOk) == "evenodd") {
	path.setFillRule(Qt::OddEvenFill);
    }
    else {
	path.setFillRule(Qt::WindingFill);
    }

    CarveGraphicsPathItem* item = new CarveGraphicsPathItem(window,path);
    finishDecorating(item);
    item->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
    this->getFillOpacity();
    this->getStrokeLineCap();
    this->getStrokeLineJoin();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    item->setBrush(this->getFill(&bOk));
    item->setPen(this->getStroke(&bOk));
}

CarvePathElement::~CarvePathElement() {
}
