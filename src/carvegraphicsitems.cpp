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
#include "carvegraphicsitems.h"
#include "carvesvgwindow.h"
#include "carvewindow.h"
#include "carvedesignview.h"

#include <QVariant>
#include <QGraphicsSceneContextMenuEvent>

#include <iostream>
using std::cout;
using std::endl;

const char* CGI_DELETE = "Delete";

// TODO: figure out why selecting nodes already selects the appropriate node in the dom browser

QVariant CarveGraphicsRectItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    /*
    CarveSVGNode* node = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
    switch(change) {
	case ItemSelectedHasChanged:
	    if(value.toBool()) {
		window_->mainwindow()->selectNode(node);
	    }
	    break;
	default: break;
    }
    */
    return QGraphicsRectItem::itemChange(change,value);
}

void CarveGraphicsRectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}

QVariant CarveGraphicsEllipseItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsEllipseItem::itemChange(change,value);
}

void CarveGraphicsEllipseItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}

QVariant CarveGraphicsLineItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsLineItem::itemChange(change,value);
}

void CarveGraphicsLineItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}

QVariant CarveGraphicsPathItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsPathItem::itemChange(change,value);
}

void CarveGraphicsPathItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}

/*
QVariant CarveGraphicsPolygonItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsPolygonItem::itemChange(change,value);
}

void CarveGraphicsPolygonItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}
*/

QVariant CarveGraphicsImageItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    return QGraphicsPixmapItem::itemChange(change,value);
}

void CarveGraphicsImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    CarveDesignView* view = this->window_->view();
    QAction* theAction = view->contextMenu()->exec(event->screenPos());
    if(theAction == view->deleteNode) {
	CarveSVGNode* elem = reinterpret_cast<CarveSVGNode*>(this->data(0).value<void*>());
	window_->mainwindow()->deleteNode(elem);
    }
}
