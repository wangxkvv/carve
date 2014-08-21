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
#include "carveimageelement.h"

#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvewindow.h"
#include "carvescene.h"
#include "domhelper.h"
#include "carvegraphicsitems.h"

#include <iostream>
using std::cout;
using std::endl;

#include <QPixmap>

CarveImageElement::CarveImageElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgImage, parent)
{
    bool bOk = false;

    qreal x = getFloatTrait(element, "x", &bOk);
    if(!bOk) { x = 0.0; }

    qreal y = getFloatTrait(element, "y", &bOk);
    if(!bOk) { y = 0.0; }

    qreal w = getFloatTrait(element, "width", &bOk);
    if(!bOk) { w = 0; }
    else if(w < 0) {
	w = 0;
	cout << "Error!  Width of image is negative" << endl;
    }

    qreal h = getFloatTrait(element, "height", &bOk);
    if(!bOk) { h = 0; }
    else if(h < 0) {
	h = 0;
	cout << "Error!  Height of image is negative" << endl;
    }

    QPixmap pix;

    QString href = getTrait(element, "xlink:href", &bOk);
    if(!href.isEmpty()) {
	Qt::AspectRatioMode arm = getAspectRatio(element);

	// just try to fetch the image...
        pix = QPixmap(href).scaled((int)w,(int)h,arm);
    }

    CarveGraphicsImageItem* item = new CarveGraphicsImageItem(window, pix);
    finishDecorating(item);
    item->setPos(x,y);

    item->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
}

CarveImageElement::~CarveImageElement() {
}
