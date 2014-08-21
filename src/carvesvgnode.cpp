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

/*
    This is the basis of all SVG elements in the editor.  Once the actual SVG document's
    text has been parsed into a DOM document, each item is represented as a CarveSVGNode.

    The CarveSVGNode contains the QDomElement object, but the CarveSVGNode also participates
    in the Model-View architecture for things like the DOM Browser.

*/

#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvesvgdocument.h"
#include "carvescene.h"

#include <QBrush>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QDomElement>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

#include "domhelper.h"

#include <iostream>
using std::cout;
using std::endl;

CarveSVGNode::CarveSVGNode(const QDomDocument& doc, int row, CarveSVGWindow* window, SvgNodeType type, CarveSVGNode* parent) :
	gfxItem_(NULL),
	window_(window),
	type_(type),
	domElem_(QDomElement()), // Set it to Null
	row_(row),
	parent_(parent)
{
    children_[0] = CarveSVGNode::createNode(doc.documentElement(), 0, this->window_, this);
}

CarveSVGNode::CarveSVGNode(const QDomElement& element, int row, CarveSVGWindow* window, SvgNodeType type, CarveSVGNode* parent) :
	gfxItem_(NULL),
	window_(window),
	type_(type),
	domElem_(element),
        row_(row),
	parent_(parent)
{
}

CarveSVGNode::~CarveSVGNode() {
    QHash<int, CarveSVGNode*>::iterator it = children_.begin();
    while(it != children_.end()) {
        delete it.value();
        ++it;
    }
    children_.clear();
}

QDomElement CarveSVGNode::domElem() const {
    return this->domElem_;
}

int CarveSVGNode::row() const {
    return this->row_;
}

CarveSVGNode* CarveSVGNode::parent() {
    return this->parent_;
}

CarveSVGNode* CarveSVGNode::child(int i) {
    // return the cached child node
    if(children_.contains(i)) {
        return children_[i];
    }

    // our children array does not yet contain a CarveSVGNode object for this index
    // so we now lazily create it
    if (i >= 0 && i < domElem_.childNodes().count()) {
	QDomNode childNode = domElem_.childNodes().item(i);
	QDomElement childElem(childNode.toElement());
	if(!childElem.isNull()) {
	    CarveSVGNode* childItem = CarveSVGNode::createNode(childElem, i, this->window_, this);
	    if(childItem->gfxItem()) {
		// TODO: do it here?
//	    this->window_->scene()->addItem(childItem->gfxItem());
	    }
	    children_[i] = childItem;
	    return childItem;
	}
    }

    // else, return NULL
    return 0;
}

// Something in the editor has changed an attribute value on this node
// This function causes the DOM to be reserialized and the editor to re-sync itself
// If the attribute's new value is an empty string, the attribute is removed from the DOM
bool CarveSVGNode::setTrait(const QString& name, const QString& value) {
    bool bResult = false;
    if(!domElem_.isNull() && ::setTrait(domElem_, name, value)) {
	// if the value of the attribute is now the empty string, we can just remove
	// the attribute so our markup stays clean
	if(value.isEmpty()) {
	    domElem_.removeAttribute(name);
	}

	// serialize the DOM and set the document's text (but make it undo-able)
	QTextCursor cursor(window_->edit()->document());
	cursor.beginEditBlock();
	cursor.select(QTextCursor::Document);
	cursor.removeSelectedText();
	// insert the reserialized DOM
	// TODO: do a diff between the previous text contents and the reserialized DOM
	// and then only change this text (this should reduce memory considerably)
	// TODO: make this default indentation a setting
	cursor.insertText(window_->model()->domDocument()->toString(2));
	cursor.endEditBlock();

	this->window_->updateDocImmediately();
    }
    else {
	cout << "Not reset" << endl;
    }

    return bResult;
}

QRegExp uri("url\\(\\#([\\w]+)\\)");

qreal CarveSVGNode::getFillOpacity() {
    bool bOk = false;
    this->fillOpacity_ = 1.0;

    qreal tempFillOpacity = 1.0;

    // first fetch from the ugly style attribute
    if(styles_.contains("fill-opacity")) {
	tempFillOpacity = styles_.value("fill-opacity").toDouble(&bOk);
    }
    // if the parse failed, then try the attribute
    if(!bOk) {
	tempFillOpacity = getFloatTrait(this->domElem(), "fill-opacity", &bOk);
    }

    // TODO: Check SVG spec, is a negative or >1 value for fill-opacity cause it be 1.0 or inherited?
    // (ensure that all inheritable properties follow the same convention, currently strokeWidth does the opposite of this function)
    if(!bOk) {
	// if we have a parent, get the fill-opacity from the parent
	if(this->parent()) {
	    this->fillOpacity_ = this->parent()->getFillOpacity();
	}
	else {
	    this->fillOpacity_ = 1.0;
	}
    }
    else {
	if(tempFillOpacity < 0.0 || tempFillOpacity > 1.0) { tempFillOpacity = 1.0; }
	this->fillOpacity_ = tempFillOpacity;
    }
    return this->fillOpacity_;
}

qreal CarveSVGNode::getStrokeOpacity() {
    bool bOk = false;
    this->strokeOpacity_ = 1.0;

    qreal tempStrokeOpacity = 1.0;
    if(styles_.contains("stroke-opacity")) {
	tempStrokeOpacity = styles_.value("stroke-opacity").toDouble(&bOk);
    }
    if(!bOk) {
	tempStrokeOpacity = getFloatTrait(this->domElem(), "stroke-opacity", &bOk);
    }

    if(!bOk) {
	// if we have a parent, get the stroke-opacity from the parent
	if(this->parent()) {
	    this->strokeOpacity_ = this->parent()->getStrokeOpacity();
	}
	else {
	    this->strokeOpacity_ = 1.0;
	}
    }
    else {
	if(tempStrokeOpacity < 0.0 || tempStrokeOpacity > 1.0) { tempStrokeOpacity = 1.0; }
	this->strokeOpacity_ = tempStrokeOpacity;
    }

    return this->strokeOpacity_;
}

// TODO: convert font-size into something I can use here
// (ok values are things like "12pt", "10px", "3em", "2ex", 14.4, "medium", etc).
// see http://www.w3.org/TR/2006/REC-xsl11-20061205/#font-size
qreal CarveSVGNode::getFontSize() {
    bool bOk = false;

    QFont dummyFont;
    this->fontSize_ = dummyFont.pointSizeF();

    qreal tempFontSize = dummyFont.pointSizeF();
    if(styles_.contains("font-size")) {
	tempFontSize = styles_.value("font-size").toDouble(&bOk);
    }
    if(!bOk) {
	tempFontSize = getFloatTrait(this->domElem(), "font-size", &bOk);
    }

    if(!bOk || tempFontSize < 0.0) {
	// if it didn't parse, it acts like an automatic inherit
	if(this->parent()) {
	    this->fontSize_ = this->parent()->getFontSize();
	}
	// if there was no parent (i.e. at <svg> node), then just use default font size from Qt (dummyFont above)
    }
    else {
	this->fontSize_ = tempFontSize;
    }

    return this->fontSize_;
}

QString CarveSVGNode::getFontFamily() {
    bool bOk = false;

    QFont dummyFont;
    this->fontFamily_ = dummyFont.family();

    QString tempFontFamily = dummyFont.family();
    if(styles_.contains("font-family")) {
	tempFontFamily = styles_.value("font-family");
    }
    if(!bOk) {
	tempFontFamily = getTrait(this->domElem(), "font-family", &bOk);
    }

    if(!bOk || tempFontFamily == "" || tempFontFamily == "inherit") {
	if(this->parent()) {
	    this->fontFamily_ = this->parent()->getFontFamily();
	}
	// if there was no parent (i.e. at <svg> node), then just use default font fmaily from Qt (dummyFont above)
    }
    else {
	this->fontFamily_ = tempFontFamily;
    }

    return this->fontFamily_;
}

qreal CarveSVGNode::getStrokeWidth() {
    bool bOk = false;
    this->strokeWidth_ = 1.0;

    qreal tempStrokeWidth = 1.0;
    if(styles_.contains("stroke-width")) {
	tempStrokeWidth = styles_.value("stroke-width").toDouble(&bOk);
    }
    if(!bOk) {
	tempStrokeWidth = getFloatTrait(this->domElem(), "stroke-width", &bOk);
    }

    if(!bOk) {
	// if it didn't parse, it acts like an automatic inherit
	if(this->parent()) {
	    this->strokeWidth_ = this->parent()->getStrokeWidth();
	}
	else {
	    this->strokeWidth_ = 1.0;
	}
    }
    else {
	if(tempStrokeWidth < 0.0) { tempStrokeWidth = 1.0; }
	this->strokeWidth_ = tempStrokeWidth;
    }

    return this->strokeWidth_;
}

Qt::PenCapStyle CarveSVGNode::getStrokeLineCap() {
    // Qt default is "square"
    // SVG default is "butt" (Qt calls this "flat")
    Qt::PenCapStyle lineCapStyle = Qt::FlatCap;

    QString rawLineCap = getTrait(this->domElem(), "stroke-linecap");
    if(rawLineCap == "round") { lineCapStyle = Qt::RoundCap; }
    else if(rawLineCap == "square") { lineCapStyle = Qt::SquareCap; }

    // if it was not specified, get the parent
    if(rawLineCap == "" || rawLineCap == "inherit") {
	if(this->parent()) {
	    lineCapStyle = this->parent()->getStrokeLineCap();
	}
	// if there was no parent (i.e. at <svg> node), then just use default linecap style (flatcap above)
    }

    this->strokeLineCap_ = lineCapStyle;

    return lineCapStyle;
}

Qt::PenJoinStyle CarveSVGNode::getStrokeLineJoin() {
    // Qt default is "BevelJoin"
    // SVG default is "miter" (Qt calls this "SvgMiterJoin")
    Qt::PenJoinStyle lineJoinStyle = Qt::SvgMiterJoin;

    QString rawLineJoin = getTrait(this->domElem(), "stroke-linejoin");
    if(rawLineJoin == "round") { lineJoinStyle = Qt::RoundJoin; }
    else if(rawLineJoin == "bevel") { lineJoinStyle = Qt::BevelJoin; }

    // if it was not specified, get the parent
    if(rawLineJoin == "" || rawLineJoin == "inherit") {
	if(this->parent()) {
	    lineJoinStyle = this->parent()->getStrokeLineJoin();
	}
	// if there was no parent (i.e. at <svg> node), then just use default linejoin style (miter above)
    }

    this->strokeLineJoin_ = lineJoinStyle;

    return lineJoinStyle;
}


// TODO: implement pen style
QPen CarveSVGNode::getStroke(bool* bOk, qreal opacity, qreal width) {
    if(opacity == -1) { opacity = this->strokeOpacity_; }
    if(width == -1) { width = this->strokeWidth_; }
    if(bOk) { *bOk = false; }

    QString rawStroke;
    if(styles_.contains("stroke")) {
	rawStroke = styles_.value("stroke");
    }
    else {
	rawStroke = getTrait(this->domElem(), "stroke");
    }

    if(rawStroke == "none") {
	if(bOk) { *bOk = true; }
	return QPen(Qt::NoPen);
    }
    else if(uri.exactMatch(rawStroke)) {
	if(uri.capturedTexts().length() == 2) {
	    // seek out the referenced element in the DOM document
	    QDomElement paintServer = getElementById(domElem().ownerDocument().documentElement(), uri.capturedTexts().at(1));

	    if(!paintServer.isNull()) {
		QString nodeName = paintServer.nodeName();
		if(nodeName == "linearGradient") {
		    if(bOk) { *bOk = true; }

		    // spec says that if no stops are specified, it's as if 'none' were specified
		    QLinearGradient g = resolveLinearGradient(paintServer, opacity);
		    QGradientStops stops = g.stops();
		    if(stops.size() == 0) {
			return QPen(QBrush(Qt::NoBrush), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		    }
		    // spec says that if only 1 stop is specified, paint a solid color (ignoring opacity, I guess)
		    else if(stops.size() == 1) {
			return QPen(QBrush(stops.at(0).second), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		    }
		    // assign gradient
		    return QPen(QBrush(g), width);
		} // linearGradient
		else if(nodeName == "radialGradient") {
		    if(bOk) { *bOk = true; }

		    QRadialGradient g = resolveRadialGradient(paintServer, opacity);
		    QGradientStops stops = g.stops();

		    // spec says that if no stops are specified, it's as if 'none' were specified
		    if(stops.size() == 0) {
			return QPen(QBrush(Qt::NoBrush), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		    }
		    // spec says that if only 1 stop is specified, paint a solid color (ignoring opacity, I guess)
		    else if(stops.size() == 1) {
			return QPen(QBrush(stops.at(0).second), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		    }

		    // assign gradient
		    return QPen(QBrush(g), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		} // radialGradient
		else if(nodeName == "solidColor") {
		    // TODO: test this
		    QBrush stroke(getRGBColorTrait(paintServer, "solid-color", bOk));
		    QColor color(stroke.color());
                    color.setAlpha((int)(opacity*255.0));
		    stroke.setColor(color);
		    return QPen(stroke, width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
		} // solidColor
	    }
	}
	else {
	    cout << "How did we capture more text?" << endl;
	    return QPen(QBrush(Qt::NoBrush), width, Qt::SolidLine, this->strokeLineCap_, this->strokeLineJoin_);
	}
    }

    // Just get the color value of the stroke (could be an empty attribute or unspecified also)
    QPen stroke;
    QBrush strokeBrush;
    if(!rawStroke.isEmpty()) {
	strokeBrush = getRGBColorTrait(rawStroke, bOk);
    }
    if(bOk && !(*bOk)) {
	// If we reach here, then the stroke attribute was not specified or did not parse or was "inherit"
	if(this->parent()) {
	    QPen parentStroke = this->parent()->getStroke(bOk, opacity, width);
	    parentStroke.setCapStyle(this->strokeLineCap_);
	    parentStroke.setJoinStyle(this->strokeLineJoin_);
	    return parentStroke;
	}
	else { // default stroke is none
	    stroke.setBrush(QBrush());
	    return stroke;
	}
    }
    // else, we parsed the RGB value ok and apply opacity and width
    stroke.setBrush(strokeBrush);

    // Now we apply the proper stroke-opacity, stroke-width, stroke-linecap
    if(opacity < 1.0 && stroke.brush().style() == Qt::SolidPattern) {
	QColor scolor(stroke.brush().color());
	scolor.setAlpha((int)(opacity*255.0));
	stroke.brush().setColor(scolor);
    }
    stroke.setWidthF(width);
    stroke.setCapStyle(this->strokeLineCap_);
    stroke.setJoinStyle(this->strokeLineJoin_);
    if(bOk) { *bOk = true; }
    return stroke;
}

QBrush CarveSVGNode::getFill(bool* bOk, qreal opacity) {
    if(opacity == -1) { opacity = this->fillOpacity_; }

    if(bOk) { *bOk = false; }

    QString rawFill;
    if(styles_.contains("fill")) {
	rawFill = styles_.value("fill");
    }
    else {
	rawFill = getTrait(this->domElem(), "fill");
    }

    if(rawFill == "none") {
	if(bOk) { *bOk = true; }
	return QBrush(Qt::NoBrush);
    }
    else if(uri.exactMatch(rawFill)) {
	if(uri.capturedTexts().length() == 2) {
	    // seek out the referenced element in the DOM document
	    // NOTE: QDomDocument::elementById() would seem to be perfect for this except for the tiny
	    // fact that THIS FUNCTION IS NOT IMPLEMENTED IN QT AND WILL ALWAYS RETURN A NULL NODE!
	    // Thus, I have to invent my own getElementById() method in domhelper.cpp
	    QDomElement paintServer = getElementById(domElem().ownerDocument().documentElement(), uri.capturedTexts().at(1));

	    if(!paintServer.isNull()) {
		QString nodeName = paintServer.nodeName();
		if(nodeName == "linearGradient") {
		    if(bOk) { *bOk = true; }

		    // TODO: handle xlink:href here
		    QLinearGradient g = resolveLinearGradient(paintServer,opacity);
		    QGradientStops stops = g.stops();

		    // spec says that if no stops are specified, it's as if 'none' were specified
		    if(stops.size() == 0) {
			return QBrush(Qt::NoBrush);
		    }
		    // spec says that if only 1 stop is specified, paint a solid color (ignoring opacity, I guess)
		    else if(stops.size() == 1) {
			return QBrush(stops.at(0).second);
		    }

		    // assign gradient
		    return QBrush(g);
		} // linearGradient
		else if(nodeName == "radialGradient") {
		    if(bOk) { *bOk = true; }

		    QRadialGradient g = resolveRadialGradient(paintServer, opacity);
		    QGradientStops stops = g.stops();

		    // spec says that if no stops are specified, it's as if 'none' were specified
		    if(stops.size() == 0) {
			return QBrush(Qt::NoBrush);
		    }
		    // spec says that if only 1 stop is specified, paint a solid color (ignoring opacity, I guess)
		    else if(stops.size() == 1) {
			return QBrush(stops.at(0).second);
		    }

		    // assign gradient
		    return QBrush(g);
		} // radialGradient
		else if(nodeName == "solidColor") {
		    // TODO: test this
		    QBrush fill(getRGBColorTrait(paintServer, "solid-color", bOk));
		    QColor color(fill.color());
                    color.setAlpha((int)(opacity*255.0));
		    fill.setColor(color);
		    return fill;
		} // solidColor
	    }
	}
	else {
	    cout << "How did we capture more text?" << endl;
	    return QBrush(Qt::NoBrush);
	}
    }

    // just get the color specified by the fill attribute (might also be empty string or "inherit" or invalid)
    QBrush fill;
    if(!rawFill.isEmpty()) {
	fill = getRGBColorTrait(rawFill, bOk);
    }
    if(bOk && !(*bOk)) {
	// If we reach here, then the fill attribute was not specified or did not parse or was "inherit"
	if(this->parent()) {
	    // since this getFill() call will populate the opacity, we return it immediately
	    return this->parent()->getFill(bOk, opacity);
	}
	else { // default is solid black fill
	    fill = QBrush(QColor("black"));
	}
    }

    // if fill opacity was specified, then adjust the fill brush by it
    if(opacity < 1.0 && fill.style() == Qt::SolidPattern) {
	QColor fcolor(fill.color());
	fcolor.setAlpha((int)(opacity*255.0));
	fill.setColor(fcolor);
    }
    return fill;
}

void CarveSVGNode::finishDecorating(QGraphicsItem* item) {
    // set up id tooltip
    QDomNode idAttr = this->domElem().attributes().namedItem("id");
    if(!idAttr.isNull()) {
	item->setToolTip(idAttr.nodeValue());
    }

    item->setZValue(this->row());
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    item->setTransform(getTransform(this->domElem()));

    // parse the style attribute and store the property/value pairs for this element
    this->getStyles();

    // add this item to its parent
    CarveSVGNode* theParent = this->parent();
    if(theParent) {
	item->setParentItem(theParent->gfxItem());

	// if the parent is a <g> or an <a>, then we need to expand
	// the parent's bounding box (a rect)
	if(theParent->type() == svgG || theParent->type() == svgA) {
	    QGraphicsRectItem* parentBox = dynamic_cast<QGraphicsRectItem*>(theParent->gfxItem());
	    if(parentBox) {
		QRectF bbox(parentBox->rect());
		QRectF itemBox(item->boundingRect());
		if(bbox.width() == -1 && bbox.height() == -1) {
		    bbox = itemBox;
		}
		else {
		    if(bbox.left() > itemBox.left()) { bbox.setLeft(itemBox.left()); }
		    if(bbox.top() > itemBox.top()) { bbox.setTop(itemBox.top()); }
		    if(bbox.right() < itemBox.right()) { bbox.setRight(itemBox.right()); }
		    if(bbox.bottom() < itemBox.bottom()) { bbox.setBottom(itemBox.bottom()); }
		}
		parentBox->setRect(bbox);
	    }
	}
    }

    this->gfxItem_ = item;
}

void CarveSVGNode::getStyles() {
    QDomElement elem = this->domElem();
    if(elem.isNull()) { return; }

    QString sStyle = getTrait(elem, "style");
    if(sStyle.length() > 0) {
	QStringList pairs = sStyle.split(";", QString::SkipEmptyParts);
	for(int p = 0; p < pairs.length(); ++p) {
	    QStringList namevalue = pairs.at(p).trimmed().split(":");
	    if(namevalue.length() == 2) {
		this->styles_[namevalue.at(0).trimmed()] = namevalue.at(1).trimmed();
	    }
	}
    }
}
