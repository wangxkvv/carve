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
#ifndef CARVESVGNODE_H
#define CARVESVGNODE_H

#include <QDomElement>
#include <QHash>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QGraphicsRectItem>

class QGraphicsItem;
class QAbstractGraphicsShapeItem;
class CarveSVGWindow;

enum SvgNodeType {
    svgUndefined,
    // actual subclasses of CarveSVGNode
    svgSvg, svgG, svgRect, svgCircle, svgEllipse, svgLine, svgPolyline, svgPolygon, svgPath, svgText, svgA, svgImage,
    // instances of CarveSVGNode (never displayed in Design mode)
    svgLinearGradient, svgRadialGradient, svgStop, svgDefs,
    // Not implemented yet
    svgUse, svgAudio, svgVideo,
};

class CarveSVGNode
{
public:
    virtual ~CarveSVGNode();

    QDomElement domElem() const;
    int row() const;
    CarveSVGNode* parent();
    CarveSVGNode* child(int i);
    int numChildren() const { return children_.size(); }
    SvgNodeType type() const { return type_; }
    QGraphicsItem* gfxItem() { return gfxItem_; }

    qreal fillOpacity() const { return fillOpacity_; }
    qreal strokeOpacity() const { return strokeOpacity_; }
    qreal strokeWidth() const { return strokeWidth_; }
    // NOTE: these anly make sense for <svg>, <g>, <a>, <text>, <tspan> - still store them here?
    qreal fontSize() const { return fontSize_; }
    QString fontFamily() const { return fontFamily_; }

    // factory methods
    static CarveSVGNode* createNode(const QDomDocument& doc, int row, CarveSVGWindow* window, CarveSVGNode* parent = 0);
    static CarveSVGNode* createNode(const QDomElement& elem, int row, CarveSVGWindow* window, CarveSVGNode* parent = 0);

    bool setTrait(const QString& name, const QString& value);

protected:
    CarveSVGNode(const QDomDocument& doc, int row, CarveSVGWindow* window, SvgNodeType type, CarveSVGNode* parent = 0);
    CarveSVGNode(const QDomElement& elem, int row, CarveSVGWindow* window, SvgNodeType type, CarveSVGNode* parent = 0);
    QGraphicsItem* gfxItem_;
    CarveSVGWindow* window_;
    QMap<QString, QString> styles_;

    qreal getFillOpacity();
    QBrush getFill(bool* bOk = NULL, qreal opacity = -1);
    qreal getStrokeOpacity();
    qreal getStrokeWidth();
    Qt::PenJoinStyle getStrokeLineJoin();
    Qt::PenCapStyle getStrokeLineCap();
    QPen getStroke(bool* bOk = NULL, qreal opacity = -1, qreal width = -1);
    qreal getFontSize();
    QString getFontFamily();
    void finishDecorating(QGraphicsItem* item);
    void getStyles();

private:
    SvgNodeType type_;
    QDomElement domElem_;
    int row_;
    CarveSVGNode* parent_;
    QHash<int,CarveSVGNode*> children_;

    qreal fillOpacity_;
    QBrush fill_;

    Qt::PenCapStyle strokeLineCap_;
    Qt::PenJoinStyle strokeLineJoin_;
    qreal strokeOpacity_;
    qreal strokeWidth_;
    QPen stroke_;

    qreal fontSize_;
    QString fontFamily_;
};

#endif // CARVESVGNODE_H
