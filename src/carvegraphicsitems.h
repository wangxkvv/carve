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
#ifndef CARVEGRAPHICSITEMS_H
#define CARVEGRAPHICSITEMS_H

#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPixmapItem>

class CarveSVGWindow;

// These are lightweight classes that wrap the QGraphicsItem subclasses
// so we can deal with when they have been selected, right-clicked, etc

class CarveGraphicsRectItem : public QGraphicsRectItem {
public:
    CarveGraphicsRectItem(CarveSVGWindow* window, QGraphicsItem * parent = 0 ) : QGraphicsRectItem(parent), window_(window) {}
    CarveGraphicsRectItem(CarveSVGWindow* window, const QRectF & rect, QGraphicsItem * parent = 0 ) : QGraphicsRectItem(rect,parent), window_(window) {}
    CarveGraphicsRectItem(CarveSVGWindow* window, qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0 ) : QGraphicsRectItem(x,y,width,height,parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};

class CarveGraphicsEllipseItem : public QGraphicsEllipseItem {
public:
    CarveGraphicsEllipseItem(CarveSVGWindow* window, QGraphicsItem * parent = 0 ) : QGraphicsEllipseItem(parent), window_(window) {}
    CarveGraphicsEllipseItem(CarveSVGWindow* window, const QRectF & rect, QGraphicsItem * parent = 0 ) : QGraphicsEllipseItem(rect,parent), window_(window) {}
    CarveGraphicsEllipseItem(CarveSVGWindow* window, qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0 ) : QGraphicsEllipseItem(x,y,width,height,parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};

class CarveGraphicsLineItem : public QGraphicsLineItem {
public:
    CarveGraphicsLineItem(CarveSVGWindow* window, QGraphicsItem * parent = 0) : QGraphicsLineItem(parent), window_(window) {}
    CarveGraphicsLineItem(CarveSVGWindow* window, const QLineF & line, QGraphicsItem * parent = 0) : QGraphicsLineItem(line,parent), window_(window) {}
    CarveGraphicsLineItem(CarveSVGWindow* window, qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem * parent = 0) : QGraphicsLineItem(x1,y1,x2,y2,parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};

class CarveGraphicsPathItem : public QGraphicsPathItem {
public:
    CarveGraphicsPathItem(CarveSVGWindow* window, QGraphicsItem * parent = 0) : QGraphicsPathItem(parent), window_(window) {}
    CarveGraphicsPathItem(CarveSVGWindow* window, const QPainterPath & path, QGraphicsItem * parent = 0) : QGraphicsPathItem(path,parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};

/*
class CarveGraphicsPolygonItem : public QGraphicsPolygonItem {
public:
    CarveGraphicsPolygonItem(CarveSVGWindow* window, QGraphicsItem * parent = 0) : QGraphicsPolygonItem(parent), window_(window) {}
    CarveGraphicsPolygonItem(CarveSVGWindow* window, const QPolygonF & polygon, QGraphicsItem * parent = 0) : QGraphicsPolygonItem(polygon,parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};
*/

class CarveGraphicsImageItem : public QGraphicsPixmapItem {
public:
    CarveGraphicsImageItem(CarveSVGWindow* window, QGraphicsItem* parent = 0) : QGraphicsPixmapItem(parent), window_(window) {}
    CarveGraphicsImageItem(CarveSVGWindow* window, const QPixmap& pixmap, QGraphicsItem* parent = 0) :
	    QGraphicsPixmapItem(pixmap, parent), window_(window) {}
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
private:
    CarveSVGWindow* window_;
};

#endif // CARVEGRAPHICSITEMS_H
