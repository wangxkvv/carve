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
#include "carvesvgelement.h"
#include "carvesvgwindow.h"
#include "carvesvgnode.h"
#include "carvescene.h"
#include "carvedesignview.h"
#include "domhelper.h"

#include <QGraphicsRectItem>

#include <iostream>
using std::cout;
using std::endl;

/*

  This is an important element - it establishes what size the scene should take up...

  If width/height are specified, determine their size in pixels:
  - no unit - pixels
  - px - pixels
  - cm/mm/in - get device information and map to pixel values
  - em - font-size on svg element
  - ex - x-height on svg element

  For now, Carve will only support unitless or percentages (100% if unspecified).

  If absolute, then we can set the scene rect and svg rect item to this value.

  If percentage, then we need to control the view's matrix (or possibly fitToSize()?)

  preserveAspectRatio - not dealing with this in the first iteration

 */
CarveSVGElement::CarveSVGElement(const QDomElement& element, int row, CarveSVGWindow* window, CarveSVGNode* parent) :
	CarveSVGNode(element, row, window, svgSvg, parent)
{
    bool bOk = false;
    CarveScene* scene = this->window_->scene();

    width_ = getFloatTrait(element, "width", &bOk);
    // width="100%" is default
    if(!bOk) {
	width_ = getPercentageTrait(element, "width", &bOk);
	if(!bOk) { width_ = 100.0; }
	// relative values are negative
	width_ = -width_;
    }

    height_ = getFloatTrait(element, "height", &bOk);
    // height="100%" is default
    if(!bOk) {
	height_ = getPercentageTrait(element, "height", &bOk);
	if(!bOk) { height_ = 100.0; }
	// relative values are negative
	height_ = -height_;
    }

    // viewBox is a whitespace/comma-separated list of 4 numbers: x, y, width, height
    // negative values of width/height mean error
    // if width==0 or height==0, disabled rendering
//    this->preserveAspectRatio_ = none;
    this->viewBox_ = QRectF(0,0,-1,-1);
    QStringList strings = getListTrait(element, "viewBox", &bOk);
    if(bOk && strings.length() == 4) {
	bool bx, by, bw, bh;
	qreal x = strings[0].toDouble(&bx);
	qreal y = strings[1].toDouble(&by);
	qreal w = strings[2].toDouble(&bw);
	qreal h = strings[3].toDouble(&bh);
	// viewBox parsed ok
	if(w > 0 && h > 0 && bx && by && bw && bh) {
	    this->viewBox_ = QRectF(x,y,w,h);
	    // parse preserveAspectRatio attribute only if viewBox has been provided
	    arm_ = getAspectRatio(element);
	}
    }

    // Set up graphical items for the <svg> element.
    canvas_ = new QGraphicsRectItem(0,0,width_,height_);
    canvas_->setBrush(QBrush(QColor(255,255,255)));
    canvas_->setPen(QPen(QColor("transparent")));
    this->getFillOpacity();
    this->getStrokeOpacity();
    this->getStrokeWidth();
    canvas_->setData(0, qVariantFromValue(reinterpret_cast<void*>(this)));
    canvas_->setZValue(0);

    viewboxCanvas_ = new QGraphicsRectItem(0,0,width_,height_);
    viewboxCanvas_->setBrush(QBrush(QColor("transparent")));
    viewboxCanvas_->setPen(QPen(QColor("transparent")));
    viewboxCanvas_->setZValue(0);
    viewboxCanvas_->setParentItem(canvas_);
    // the <svg> element has the viewbox canvas as its item to which child element get attached
    this->gfxItem_ = viewboxCanvas_;

    scene->addItem(canvas_);
}

CarveSVGElement::~CarveSVGElement() {
}

// viewWidth and viewHeight are the actual size of the view widget/pane
void CarveSVGElement::update(int viewWidth, int viewHeight) {
    // if our width and height values are negative here, that means we have percentage values
    // so set the width/height to be a percentage of the view's dimension
    qreal w = width_;
    qreal h = height_;
    if(w < 0) {
	w = (-width_*viewWidth/100.0);
    }
    if(h < 0) {
	h = (-height_*viewHeight/100.0);
    }

    canvas_->setRect(0, 0, w, h);

    // TODO: handle when vb has zero width/height specified (spec says the element is not rendered)
    qreal vbx = this->viewBox_.x();
    qreal vby = this->viewBox_.y();
    qreal vbw = this->viewBox_.width();
    qreal vbh = this->viewBox_.height();
    if(vbw > 0 && vbh > 0) {
	qreal sx = w/(vbw);
	qreal sy = h/(vbh);
	qreal tx = vbx;
	qreal ty = vby;

	QTransform t;

	if(this->arm_ != Qt::IgnoreAspectRatio) {
	    if(sx > sy) {
		if(this->arm_ == Qt::KeepAspectRatio) { sx = sy; }
		else { sy = sx; }
	    }
	    else {
		if(this->arm_ == Qt::KeepAspectRatio) { sy = sx; }
		else { sx = sy; }
	    }

	    // NOTE: as long as it's not 'none' we know we need to preserve the aspect ratio
	    // but we don't really need to worry about what specific align value in an editor
	    // (user just has a canvas they can scroll around in, we don't need to tag it to an
	    //  individual corner or center it here)
	} // preserveAspectRatio align value

	t.scale(sx,sy);
	t.translate(-tx,-ty);
	viewboxCanvas_->setTransform(t);
    }


}
