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
#include "carverectelement.h"
#include "carvesvgwindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>

#include <iostream>
using std::cout;
using std::endl;

CarveRectElement::CarveRectElement(const QDomElement& node, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
    CarveSVGNode(node, row, window, svgRect, parent)
{
    bool bOk = false;

    qreal x = getFloatTrait(node, "x",&bOk);
    if(!bOk) {
	x = 0.0;
    }

    qreal y = getFloatTrait(node, "y",&bOk);
    if(!bOk) {
	y = 0.0;
    }

    qreal width = getFloatTrait(node, "width",&bOk);
    if(!bOk) {
	width = 0.0;
    }

    qreal height = getFloatTrait(node, "height",&bOk);
    if(!bOk) {
	height = 0.0;
    }

    CarveGraphicsRectItem* item = new CarveGraphicsRectItem(window, x, y, width, height);
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

CarveRectElement::~CarveRectElement() {
}
