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
#include "carveaelement.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsRectItem>

CarveAElement::CarveAElement(const QDomElement& node, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(node, row, window, svgA, parent)
{
    CarveGraphicsRectItem* a = new CarveGraphicsRectItem(window,0,0,0,0);
    finishDecorating(a);
    this->getFillOpacity();
    this->getStrokeLineCap();
    this->getStrokeLineJoin();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    a->setBrush(QBrush(QColor("transparent")));
    a->setPen(QPen(QColor("transparent")));
}

CarveAElement::~CarveAElement() {
}
