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
#ifndef DOMHELPER_H
#define DOMHELPER_H

#include <QDomNode>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QColor>
#include <QBrush>
#include <QPainterPath>
#include <QPen>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QStack>

QDomElement getElementById(const QDomElement& element, const QString& id);

// trait access
QString getTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
double getFloatTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
QBrush getRGBColorTrait(const QString& colorString, bool* bOk = NULL);
QBrush getRGBColorTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
QStringList getListTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
double getPercentageTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
QPainterPath getPathTrait(const QDomElement& element, const QString& name, bool* bOk = NULL);
QTransform getTransform(const QDomElement& element, bool* bOk = NULL);
bool setTrait(QDomElement& element, const QString& name, const QString& value);
QGradientStops fetchGradientStops(QDomElement paintServer, qreal opacity);
QLinearGradient resolveLinearGradient(const QDomElement& element, qreal opacity, QStack<QString> referenceStack = QStack<QString>());
QRadialGradient resolveRadialGradient(const QDomElement& element, qreal opacity, QStack<QString> referenceStack = QStack<QString>());
Qt::AspectRatioMode getAspectRatio(const QDomElement& element);

#endif // DOMHELPER_H
