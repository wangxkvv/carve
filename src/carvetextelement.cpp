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

#include "carvetextelement.h"
#include "domhelper.h"

#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QFontMetrics>

#include <iostream>
using std::cout;
using std::endl;

CarveTextElement::CarveTextElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgText, parent)
{
    bool bOk = false;

    qreal x = getFloatTrait(element, "x",&bOk);
    if(!bOk) {
	x = 0.0;
    }

    qreal y = getFloatTrait(element, "y",&bOk);
    if(!bOk) {
	y = 0.0;
    }

    // TODO: handle tspan, a, etc inside the text element
    QString textContents = element.text();

    QGraphicsSimpleTextItem* item = new QGraphicsSimpleTextItem(textContents);
    item->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
    item->setZValue(this->row());
    item->setTransform(getTransform(this->domElem()));
    item->setParentItem(this->parent()->gfxItem());    
    QDomNode idAttr = this->domElem().attributes().namedItem("id");
    if(!idAttr.isNull()) {
	item->setToolTip(idAttr.nodeValue());
    }
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);

    // TODO: handle text-anchor
    // TODO: handle direction
    // TODO: handle font-weight
    // TODO: handle font-style
    // TODO: handle font-variant
    QFont theFont;

    theFont.setFamily(this->getFontFamily());
    theFont.setPointSizeF(this->getFontSize());

    item->setFont(theFont);

    QFontMetrics metrics(theFont);

    // TODO: properly position this w.r.t the baseline
    item->setPos(x,y - metrics.ascent());

    this->getFillOpacity();
    this->getStrokeLineCap();
    this->getStrokeLineJoin();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    item->setBrush(this->getFill());
    item->setPen(this->getStroke());
    this->gfxItem_ = item;

}

CarveTextElement::~CarveTextElement() {
}
