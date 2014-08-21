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
#include "carvepolylineelement.h"
#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <QDomNode>
#include <QGraphicsPathItem>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>

#include <iostream>
using std::cout;
using std::endl;

CarvePolylineElement::CarvePolylineElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgPolyline, parent)
{
    bool bOk = false;
    bool bInvalidCoords = false;
    QPainterPath path;
    QColor fill, stroke;
    double startx = 0, starty = 0;
    QStringList coords(getListTrait(element, "points", &bOk));
    // must parse ok, must have an even number of coordinates
    if(bOk && coords.size() > 0 && coords.size() % 2 == 0) {
	for(int i = 0; i < coords.size() && !bInvalidCoords; i += 2) {
	    bool xok, yok;
	    double x = coords.at(i).toDouble(&xok);
	    double y = coords.at(i+1).toDouble(&yok);

	    if(xok && yok) {
		if(i == 0) {
		    startx = x;
		    starty = y;
		    path.moveTo(x,y);
		}
		else {
		    path.lineTo(x,y);
		}
	    }
	    else {
		bInvalidCoords = true;
	    }
	}

	if(!bInvalidCoords) {
	    path.moveTo(startx, starty);
	}
	else {
	    // clear any points we accumulated
	    path = QPainterPath();
	}
    }

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

CarvePolylineElement::~CarvePolylineElement() {
}
