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
#include "carvecircleelement.h"
#include "carvesvgwindow.h"
#include "carvewindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>

CarveCircleElement::CarveCircleElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgCircle, parent)
{
    bool bOk = false;

    qreal cx = getFloatTrait(element, "cx",&bOk);
    if(!bOk) {
	cx = 0.0;
    }

    qreal cy = getFloatTrait(element, "cy",&bOk);
    if(!bOk) {
	cy = 0.0;
    }

    qreal radius = getFloatTrait(element, "r",&bOk);
    if(!bOk) {
	radius = 0.0;
    }

    CarveGraphicsEllipseItem* item = new CarveGraphicsEllipseItem(window, cx-radius, cy-radius, radius*2, radius*2);
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

CarveCircleElement::~CarveCircleElement() {
}
