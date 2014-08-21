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
#include "carvesvgnode.h"
#include "carveellipseelement.h"
#include "carvesvgwindow.h"
#include "carvewindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>

#include <iostream>
using std::cout;
using std::endl;

CarveEllipseElement::CarveEllipseElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgEllipse, parent)
{
    bool bOk = false;

    qreal cx = getFloatTrait(element, "cx",&bOk);
    if(!bOk) { cx = 0.0; }

    qreal cy = getFloatTrait(element, "cy",&bOk);
    if(!bOk) { cy = 0.0; }

    qreal rx = getFloatTrait(element, "rx",&bOk);
    if(!bOk) { rx = 0.0; }

    qreal ry = getFloatTrait(element, "ry",&bOk);
    if(!bOk) { ry = 0.0; }

    CarveGraphicsEllipseItem* item = new CarveGraphicsEllipseItem(window, cx-rx, cy-ry, rx*2, ry*2);
    finishDecorating(item);
    item->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
    this->getFillOpacity();
    this->getStrokeLineCap();
    this->getStrokeLineJoin();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    item->setBrush(this->getFill());
    item->setPen(this->getStroke());
}

CarveEllipseElement::~CarveEllipseElement() {
}
