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

#include "domhelper.h"

#include <QDomNode>
#include <QString>
#include <QDomElement>
#include <QColor>
#include <QBrush>
#include <QPainterPath>
#include <QPen>
#include <QGradientStops>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QTextCursor>
#include <cmath>

#include <iostream>
using std::cout;
using std::endl;

QRegExp commaOrWS("(\\s*\\,\\s*)|\\s+");
QRegExp coord("\\-?\\d+\\.?\\d*");
QRegExp pct("(\\d+\\.?\\d*)\\%");
const qreal PI = 3.14159265358979;

QDomElement getElementById(const QDomElement& element, const QString& id) {
    if(element.isNull()) { return QDomElement(); }
    if(id.isEmpty() || id.isNull()) { return QDomElement(); }

    if(element.attribute("id") == id) {
	return element;
    }

    // else, iterate recursively over children
    for(unsigned int i = 0; i < element.childNodes().length(); ++i) {
	QDomElement child = element.childNodes().at(i).toElement();
	if(!child.isNull()) {
	    QDomElement findElem = getElementById(child, id);
	    if(!findElem.isNull()) { return findElem; }
	}
    }
    return QDomElement();
}

QString getTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return QString(); }
    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	if(bOk) { *bOk = true; }
	return attrNode.nodeValue();
    }
    return QString();
}

double getFloatTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return 0.0; }
    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	return attrNode.nodeValue().toDouble(bOk);
    }
    return 0.0;
}

double getPercentageTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return 0.0; }
    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	if(pct.exactMatch(attrNode.nodeValue())) {
	    QStringList strings(pct.capturedTexts());
	    if(strings.size() == 2) {
		return strings.at(1).toDouble(bOk);
	    }
	}
    }
    return 0.0;
}

QBrush getRGBColorTrait(const QString& colorString, bool* bOk) {
    // TODO: if we match against a url(#name) then try to find that reference first
    // (if it's a gradient or if it's a solidColor element then return QColor() and false?)

    // TODO: this reports an error on std::out or std::err if it's not of the following
    // formats: #RGB, #RRGGBB, or one of the SVG color names.  How to shut this off?
    QColor color(colorString);
    if(color.isValid()) {
	if(bOk) { *bOk = true; }
	return QBrush(color);
    }

    // else, need to try and parse the "rgb(r,g,b)" format and return that color if found
    // From http://www.w3.org/TR/SVGTiny12/painting.html#colorSyntax
    bool bR, bG, bB;

    // TODO: what whitespace rules should I follow here?  sprinkle some \\s* around?
    // Integer functional:  rgb(rrr,ggg,bbb)
    QRegExp intfn("^rgb\\(\\s*(\\d{1,3})\\s*\\,\\s*(\\d{1,3})\\s*\\,\\s*(\\d{1,3})\\)$");
    if(intfn.exactMatch(colorString)) {
	QStringList strings(intfn.capturedTexts());
	int r = strings.at(1).toInt(&bR);
	int g = strings.at(2).toInt(&bG);
	int b = strings.at(3).toInt(&bB);
	if(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255 && bR && bG && bB) {
	    if(bOk) { *bOk = true; }
	    return QBrush(QColor(r,g,b));
	}
    }

    // Float functional:  rgb(R%, G%, B%)
    // (1 or more digits, followed by 0 or 1 decimal point, followed by 0 or more digits)
    QRegExp floatfn("^rgb\\(\\s*(\\d+\\.?\\d*)\\%\\s*\\,\\s*(\\d+\\.?\\d*)\\%\\s*\\,\\s*(\\d+\\.?\\d*)\\%\\)$");
    if(floatfn.exactMatch(colorString)) {
	QStringList strings(floatfn.capturedTexts());
	double r = strings.at(1).toDouble(&bR);
	double g = strings.at(2).toDouble(&bG);
	double b = strings.at(3).toDouble(&bB);
	if(r >= 0 && r <= 100 && g >= 0 && g <= 100 && b >= 0 && b <= 100 && bR && bG && bB) {
	    if(bOk) { *bOk = true; }
            return QBrush(QColor((int)((r/100.0)*255),(int)((g/100.0)*255),(int)((b/100.0)*255)));
	}
    }
    return QBrush();
}

QBrush getRGBColorTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return QBrush(); }
    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	return getRGBColorTrait(attrNode.nodeValue(), bOk);
    }
    // return an invalid color (caller must decide what to do then)
    return QBrush();
}

QStringList getListTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return QStringList(); }
    QStringList strings;
    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	// parse this into a whitespace and/or comma-separated list of numbers
	if(bOk) { *bOk = true; }
	return attrNode.nodeValue().trimmed().split(commaOrWS);
    }
    return QStringList();
}

// This is so much fun thanks to the spec:
// Commands are one letter:  M, L, H, V, Z, C, S, Q, T, A
// command can also be lower-case (relative)
// command letters can be skipped if the same command is used multiple times in a row
// all whitespace and commas can be ignored and can be eliminated (particularly 100-100 is valid for 100,-100)
enum EPathParserState {
    START,
    IN_MOVETO, IN_CLOSEPATH, IN_LINETO, IN_HLINETO, IN_VLINETO, IN_CUBBEZTO, IN_SMOOTHCUBEBEZTO, IN_QUADBEZTO, IN_SMOOTHQUADBEZTO, IN_ELLIPARCTO,
    NONE,
    ERROR
};
QRegExp pathCmd("[MLHVZCSQTAmlhvzcsqta]");

bool bRelative = false;
int numCoordsExpected;
QList<qreal> coords;

EPathParserState findCmd(const QString& text, int& index) {
    int cmdIndex = pathCmd.indexIn(text, index);
    if(cmdIndex >= index) {
	index = cmdIndex+1;
	QChar Command = text.at(cmdIndex);
	char command = Command.toAscii();

	if(command >= 'a' && command <= 'z') { bRelative = true; command = Command.toUpper().toAscii(); }
	else { bRelative = false; }

	switch(command) {
	    case 'M': return IN_MOVETO;
	    case 'L': return IN_LINETO;
	    case 'H': return IN_HLINETO;
	    case 'V': return IN_VLINETO;
	    case 'Z': return IN_CLOSEPATH;
	    case 'C': return IN_CUBBEZTO;
	    case 'S': return IN_SMOOTHCUBEBEZTO;
	    case 'Q': return IN_QUADBEZTO;
	    case 'T': return IN_SMOOTHQUADBEZTO;
	    case 'A': return IN_ELLIPARCTO;
	    default: return NONE;
	}
    }
    return NONE;
}

int findCoords(const QString& text, int index) {
    coords.clear();

    bool bOk = false;
    int coordnum = 0;
    int cmdIndex = pathCmd.indexIn(text, index);
    int coordIndex = coord.indexIn(text, index);
    int coordLength = coord.matchedLength();

    while( (coordIndex < cmdIndex || cmdIndex == -1) && coordIndex >= index && coordLength > 0)
    {
	qreal thisCoord = text.mid(coordIndex, coordLength).toFloat(&bOk);
	if(!bOk) { break; }

	coords.append(thisCoord);
	coordnum++;
	index = coordIndex+coordLength;

	coordIndex = coord.indexIn(text, index);
	coordLength = coord.matchedLength();
    }
    return index;
}

// TODO: elliptical arc command can cause it to hang
QPainterPath getPathTrait(const QDomElement& element, const QString& name, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return QPainterPath(); }
    QPainterPath path;
    path.moveTo(0,0);

    QDomNode attrNode(element.attributes().namedItem(name));
    if(!attrNode.isNull()) {
	if(bOk) { *bOk = true; }

	QString text = attrNode.nodeValue().trimmed();

	int index = 0;
	EPathParserState state = START, prevState = START;

	qreal prevBezCtrlPtX = 0;
	qreal prevBezCtrlPtY = 0;
	QPointF subpathStart;

	// use QRegExp::indexIn(str, offset) to determine which comes first (cmd, coord)
	// map from the command type to the number of coords expected
	// when we get a command, must be able to match exactly the number of coords expected, otherwise return QPainterPath()
	// once we get those number of coords and add the path segment, determine if we have a command next or the same # of coords
	// if first command is not an absolute move (M), is it an error?
	while(index < text.size()) {
	    switch(state) {
		case START: {
		    // if first character is not 'M', go to error
		    if(text.mid(index, 1) == "M" || text.mid(index, 1) == "m") {
			state = IN_MOVETO;
			index++;
		    }
		    else {
			state = ERROR;
		    }
		    break; }
		case IN_MOVETO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need an even number of coordinates
		    // TODO: there is a bug here, only two coords are allowed for the M command, all subsequent points
		    // are considered implicit LineTo commands see http://www.w3.org/TR/SVGTiny12/paths.html#PathDataMovetoCommands
		    if(numCoords > 0 && numCoords % 2 == 0) {
			for(int n = 0; n < numCoords; n += 2) {
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    subpathStart = QPointF(x,y);
			    path.moveTo(x,y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_LINETO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need an even number of coordinates
		    if(numCoords > 0 && numCoords % 2 == 0) {
			for(int n = 0; n < numCoords; n += 2) {
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    path.lineTo(x,y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_HLINETO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    if(numCoords > 0) {
			for(int n = 0; n < numCoords; ++n) {
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y = path.currentPosition().y();
			    path.lineTo(x,y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_VLINETO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need an even number of coordinates
		    if(numCoords > 0) {
			for(int n = 0; n < numCoords; ++n) {
			    qreal x = path.currentPosition().x();
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n) : coords.at(n);
			    path.lineTo(x,y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_CLOSEPATH: {
		    path.closeSubpath();
		    // in Qt when you close a subpath, you go back to (0,0)
		    // in SVG we need to go back to the start of the previous subpath
		    path.moveTo(subpathStart);
		    state = findCmd(text, index);
		    break; }
		case IN_CUBBEZTO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need a multiple of 6 number of coordinates
		    if(numCoords > 0 && numCoords % 6 == 0) {
			for(int n = 0; n < numCoords; n += 6) {
			    qreal x1 = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y1 = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    qreal x2 = bRelative ? path.currentPosition().x() + coords.at(n+2) : coords.at(n+2);
			    qreal y2 = bRelative ? path.currentPosition().y() + coords.at(n+3) : coords.at(n+3);
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n+4) : coords.at(n+4);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+5) : coords.at(n+5);
			    path.cubicTo(x1, y1, x2, y2, x, y);
			    // do reflection of second control point around x,y
			    prevBezCtrlPtX = x-(x2-x);
			    prevBezCtrlPtY = y-(y2-y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_SMOOTHCUBEBEZTO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need a multiple of 4 number of coordinates
		    if(numCoords > 0 && numCoords % 4 == 0) {
			for(int n = 0; n < numCoords; n += 4) {
			    qreal x2 = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y2 = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n+2) : coords.at(n+2);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+3) : coords.at(n+3);
			    qreal x1 = (prevState == IN_CUBBEZTO || prevState == IN_SMOOTHCUBEBEZTO ? prevBezCtrlPtX : path.currentPosition().x());
			    qreal y1 = (prevState == IN_CUBBEZTO || prevState == IN_SMOOTHCUBEBEZTO ? prevBezCtrlPtY : path.currentPosition().y());
			    path.cubicTo(x1, y1, x2, y2, x, y);
			    // do reflection of second control point around x,y
			    prevBezCtrlPtX = x-(x2-x);
			    prevBezCtrlPtY = y-(y2-y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_QUADBEZTO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need a multiple of 4 number of coordinates
		    if(numCoords > 0 && numCoords % 4 == 0) {
			for(int n = 0; n < numCoords; n += 4) {
			    qreal x1 = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y1 = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n+2) : coords.at(n+2);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+3) : coords.at(n+3);
			    path.quadTo(x1, y1, x, y);
			    // do reflection of second control point around x,y
			    prevBezCtrlPtX = x-(x1-x);
			    prevBezCtrlPtY = y-(y1-y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		case IN_SMOOTHQUADBEZTO: {
		    // find the number of consecutive coordinates next
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    // need a multiple of 2 number of coordinates
		    if(numCoords > 0 && numCoords % 2 == 0) {
			for(int n = 0; n < numCoords; n += 2) {
			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    qreal x1 = (prevState == IN_QUADBEZTO || prevState == IN_SMOOTHQUADBEZTO ? prevBezCtrlPtX : path.currentPosition().x());
			    qreal y1 = (prevState == IN_QUADBEZTO || prevState == IN_SMOOTHQUADBEZTO ? prevBezCtrlPtY : path.currentPosition().y());
			    path.quadTo(x1, y1, x, y);
			    // do reflection of second control point around x,y
			    prevBezCtrlPtX = x-(x1-x);
			    prevBezCtrlPtY = y-(y1-y);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		// TODO: Elliptical Arc command here
		/*
		case IN_ELLIPARCTO: {
		    int newIndex = findCoords(text, index);
		    int numCoords = coords.length();
		    cout << "numCoords = " << numCoords << endl;
		    if(numCoords > 0 && numCoords % 7 == 0) {
			for(int n = 0; n < numCoords; n += 7) {
			    qreal rx = bRelative ? path.currentPosition().x() + coords.at(n) : coords.at(n);
			    cout << "rx = " << rx << endl;
			    qreal ry = bRelative ? path.currentPosition().y() + coords.at(n+1) : coords.at(n+1);
			    cout << "ry = " << ry << endl;
			    qreal xAxisRot = coords.at(n+2);
			    cout << "xAxisRot = " << xAxisRot << endl;

			    // TODO: SVG 1.1 spec doesn't say what to do if a value other than 0 or 1 is set here
			    bool largeArcFlag = (coords.at(n+3) == 1);
			    cout << "largeArcFlag = " << largeArcFlag << endl;
			    bool sweepFlag = (coords.at(n+4) == 1);
			    cout << "sweepFlag = " << sweepFlag << endl;

			    qreal x = bRelative ? path.currentPosition().x() + coords.at(n+5) : coords.at(n+5);
			    cout << "x = " << x << endl;
			    qreal y = bRelative ? path.currentPosition().y() + coords.at(n+6) : coords.at(n+6);
			    cout << "y = " << y << endl;

			    // now to make sense of this: file:///Volumes/Untitled/spec-svg11/paths.html#PathDataEllipticalArcCommands
			    // and map it to this: file:///usr/local/Trolltech/Qt-4.5.0-beta1/doc/html/qpainterpath.html#arcTo

			    // looks like xAxisRot should be dealt with separately
			    //path.arcTo(x,y,width,height,sweep);
			}
		    }
		    else {
			state = ERROR;
		    }
		    index = newIndex;
		    state = findCmd(text, index);
		    break; }
		//*/
		case ERROR: {
		    cout << "ERROR in path parsing" << endl;
		    if(bOk) { *bOk = false; }
		    return QPainterPath(); }
		case NONE: {
		    index = text.size();
		    break; }
		default: {
		    cout << "Remaining commands not supported yet" << endl;
		    index = text.size();
		    break; }
	    } // switch(state)
	    prevState = state;
	} // while(index < text.size())
    } // had the attribute

    path.closeSubpath();
    return path;
}

QRegExp coorde("[\\-\\+]?\\d+\\.?\\d*([eE][\\-\\+]?\\d+)?");
QRegExp separateExp("([\\-\\+\\d\\.]+)[eE]([\\-\\+\\d\\.]+)");
QList<qreal> getTransformValues(const QString& val, bool* bOk) {
    QList<qreal> results;
    if(bOk) { *bOk = false; }

    QStringList strings(val.trimmed().split(commaOrWS));
    if(strings.size() > 0) {
	for(int i = 0; i < strings.size(); ++i) {
	    QString num(strings.at(i));
	    if(coorde.exactMatch(num)) {
		// if it doesn't contain an exponent, parse to a qreal immediately
		if(!separateExp.exactMatch(num)) {
		    results.append(strings.at(i).toDouble(bOk));
		    if(bOk && *bOk == false) { return QList<qreal>(); }
		}
		// else, extract exponent part, parse both to qreals, do math
		else {
		    QStringList vals(separateExp.capturedTexts());
		    if(vals.length() == 3) {
			if(bOk) { *bOk = true; }
			qreal base = vals.at(1).toDouble(bOk);
			if(bOk && *bOk == false) { return QList<qreal>(); }
			qreal exp = vals.at(2).toDouble(bOk);
			if(bOk && *bOk == false) { return QList<qreal>(); }
			results.append( base * pow(10,exp) );
		    }
		    else {
			if(bOk) { *bOk = false; }
			return QList<qreal>();
		    }
		}
	    }
	}
    }
    return results;
}

QRegExp xform("\\b(translate|scale|rotate|skewX|skewY|matrix)\\b\\s*\\(([\\-\\+\\.\\d\\s\\,eE]+)\\)");
QTransform getTransform(const QDomElement& element, bool* bOk) {
    if(bOk) { *bOk = false; }
    if(element.isNull()) { return QTransform(); }

    QList<QTransform> tlist;
    QDomNode attrNode(element.attributes().namedItem("transform"));
    if(!attrNode.isNull()) {
	QString attrValue(attrNode.nodeValue());
	int index = 0;
	while((index = xform.indexIn(attrValue,index)) != -1) {
	    index += xform.matchedLength();

	    // first captured string is the transform type
	    // second captured string is the values in the parentheses (which must further be parsed)
	    QStringList strings(xform.capturedTexts());
	    if(strings.length() == 3) {
		// parse out parentheses contents
		// ...
		QString transformType = strings.at(1);
		QList<qreal> values(getTransformValues(strings.at(2).trimmed(),bOk));

		if(transformType == "translate") {
		    if(values.size() == 2) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.translate(values.at(0), values.at(1));
			tlist.append(t);
		    }
		    else if(values.size() == 1) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.translate(values.at(0), 0);
			tlist.append(t);
		    }
		}
		else if(transformType == "scale") {
		    if(values.size() == 2) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.scale(values.at(0), values.at(1));
			tlist.append(t);
		    }
		    else if(values.size() == 1) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.scale(values.at(0), values.at(0));
			tlist.append(t);
		    }
		}
		else if(transformType == "rotate") {
		    if(values.size() == 3) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.translate(values.at(1), values.at(2));
			t.rotate(values.at(0));
			t.translate(-values.at(1), -values.at(2));
			tlist.append(t);
		    }
		    else if(values.size() == 1) {
			if(bOk) { *bOk = true; }
			QTransform t;
			t.rotate(values.at(0));
			tlist.append(t);
		    }
		}
		else if(transformType == "skewX") {
		    if(values.size() == 1) {
			if(bOk) { *bOk = true; }
			QTransform t(1, 0, 0,
				    tan(values.at(0)*PI/180), 1, 0,
				    0, 0, 1);
			tlist.append(t);
		    }
		}
		else if(transformType == "skewY") {
		    if(values.size() == 1) {
			if(bOk) { *bOk = true; }
			QTransform t(1, tan(values.at(0)*PI/180), 0,
				    0, 1, 0,
				    0, 0, 1);
			tlist.append(t);
		    }
		}
		else if(transformType == "matrix") {
		    if(values.size() == 6) {
			if(bOk) { *bOk = true; }
			QTransform t(values.at(0), values.at(1), 0, values.at(2), values.at(3), 0, values.at(4), values.at(5), 1);
			tlist.append(t);
		    }
		}
		else { // invalid transform type
		    if(bOk) { *bOk = false; }
		    return QTransform();
		}
	    }
	    else {
		if(bOk) { *bOk = false; }
		return QTransform();
	    }
	}
    }

    if(tlist.size() > 0) {
	QTransform t;
	for(int i = tlist.size()-1; i >= 0; i--) {
	    t *= tlist.at(i);
	}
	return t;
    }

    return QTransform();
}

// Something in the editor has changed an attribute value on this node
// This function causes the DOM to be reserialized and the editor to re-sync itself
bool setTrait(QDomElement& element, const QString& name, const QString& value) {
    if(element.isNull()) { return false; }
    bool bResult = true;

    QDomNode attr(element.attributes().namedItem(name));
    if(!attr.isNull() && attr.isAttr()) {
	attr.setNodeValue(value);
    }
    else { // add the attribute
	element.setAttribute(name, value);
    }

    return bResult;
}

QGradientStops fetchGradientStops(QDomElement paintServer, qreal opacity) {
    QGradientStops stops;
    // get all stops inside the gradient
    QDomElement stop = paintServer.firstChildElement("stop");
    while(!stop.isNull()) {
	// get stop info here...
	bool bOff = false, bOpac = false, bColor = false;
	qreal offset = stop.attribute("offset", "0").toDouble(&bOff);
	if(!bOff || offset < 0.0 || offset > 1.0) {
	    offset = 0.0;
	}

	QColor stopColor;
	QBrush stopBrush = getRGBColorTrait(stop, "stop-color", &bColor);
	if(!bColor || stopBrush.style() == Qt::NoBrush) {
	    stopColor = QColor("black");
	}
	else {
	    stopColor = stopBrush.color();
	}

	qreal stopOpacity = getFloatTrait(stop, "stop-opacity", &bOpac);
	if(!bOpac || stopOpacity < 0 || stopOpacity > 1.0) { stopOpacity = 1.0; }

        stopColor.setAlpha((int)(stopOpacity * opacity * 255.0));

	// ensure offset is greater than or equal to the previous gradient stop's offset
	qreal prevOffset = 0.0;
	if(stops.size() > 0) { prevOffset = stops.at(stops.size()-1).first; }
	if(offset < prevOffset) { offset = prevOffset; }

	stops.append(QGradientStop(offset, stopColor));

	stop = stop.nextSiblingElement("stop");
    } // for each stop
    return stops;
}

QRegExp uriFrag("\\#([\\w]+)");

QLinearGradient resolveLinearGradient(const QDomElement& element, qreal opacity, QStack<QString> referenceStack) {
    bool bOk = false;
    // default start/stop for Qt is (0,0) and (1,1)
    // default start/stop for SVG is (0,0) and (1,0)
    QLinearGradient g(0,0,1,0);
    // default userSpaceOnUse="objectBoundingBox"
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    // default spreadMethod="pad"
    g.setSpread(QGradient::PadSpread);

    // check the xlink:href attribute on this element, if it refers to a valid linear/radial gradient
    // that is not in our referenceStack, then go fetch it and copy the valid attributes and stops
    QString href = getTrait(element, "xlink:href", &bOk);
    if(bOk && !href.isEmpty()) {
	int refIndex = uriFrag.indexIn(href);
	if(refIndex == 0) {
	    QString refID = href.mid(refIndex+1);
	    if(!referenceStack.contains(refID)) {
		// fetch the new element
		QDomElement refElem = getElementById(element.ownerDocument().documentElement(), refID);
		if(!refElem.isNull()) {
		    // determine if it's a linear/radial gradient
		    QString tagName(refElem.tagName());
		    if(tagName == "linearGradient") {
			referenceStack.push(refID);
			// recursively call the resolve function to get the gradient
			QLinearGradient refGrad = resolveLinearGradient(refElem, opacity, referenceStack);
			// apply relevant stops and attributes to g
			cout << "refGrad.end = " << refGrad.finalStop().x() << "," << refGrad.finalStop().y() << endl;
			g.setStart(refGrad.start());
			g.setFinalStop(refGrad.finalStop());
			g.setCoordinateMode(refGrad.coordinateMode());
			g.setSpread(refGrad.spread());
			g.setStops(refGrad.stops());
		    }
		    else if(tagName == "radialGradient") {
			referenceStack.push(refID);
			// recursively call the resolve function to get the gradient
			QRadialGradient refGrad = resolveRadialGradient(refElem, opacity, referenceStack);
			// apply relevant stops and attributes to g
			g.setCoordinateMode(refGrad.coordinateMode());
			g.setSpread(refGrad.spread());
		    }
		    else {
			cout << "Error! Reference '" << refID.toStdString().c_str() << "' is not a linear/radial gradient" << endl;
		    }
		}
		else {
		    cout << "Error!  Gradient reference '" << refID.toStdString().c_str() << "' not found" << endl;
		}
	    }
	    else {
		cout << "Error!  Circular reference detected with gradient '" << refID.toStdString().c_str() << "'" << endl;
	    }
	} // found a URI frag
    } // found xlink:href

    // now fetch the rest of this element's attributes and override any referenced gradient above

    // this will get the linear gradient's current x1,y1 and x2,y2
    // (which may be set by a referenced gradient)
    QPointF curStart = g.start();
    QPointF curEnd = g.finalStop();

    // TODO: handle percentages here
    bool bx1 = false, by1 = false, bx2 = false, by2 = false;
    qreal x1 = element.attribute("x1", QString("%1").arg(curStart.x())).toDouble(&bx1);
    qreal y1 = element.attribute("y1", QString("%1").arg(curStart.y())).toDouble(&by1);
    qreal x2 = element.attribute("x2", QString("%1").arg(curEnd.x())).toDouble(&bx2);
    qreal y2 = element.attribute("y2", QString("%1").arg(curEnd.y())).toDouble(&by2);
    cout << "Parsed x2,y2 = " << x2 << "," << y2 << endl;
    cout << "Flags bx2, by2 = " << bx2 << "," << by2 << endl;

    // x1,y1,x2,y2 will now be populated with some value
    // however, if x1="foo", x1 will be 0 but bx1 will be false
    // TODO: in this case, does the spec say to use the referenced values or the default values
    // test:  <linearGradient xlink:href="#grad1" id="grad2" x1="foo" />
    // Does x1 become 0 (default) or the value of grad1's x1?
    if(!bx1) { x1 = curStart.x(); }
    if(!by1) { y1 = curStart.y(); }
    if(!bx2) { x2 = curEnd.x(); }
    if(!by2) { y2 = curEnd.y(); }

    // actually set the end points
    g.setStart(x1, y1);
    g.setFinalStop(x2, y2);

    // gradientUnits attribute
    // (gradientUnits="userSpaceOnUse", objectBoundingBox is the default mode set above)
    QString gradientUnits(getTrait(element, "gradientUnits"));
    if(gradientUnits == "userSpaceOnUse") {
	g.setCoordinateMode(QGradient::LogicalMode);
    }
    // TODO: does spec say to use default value or referenced gradient's value
    // (I believe it would be referenced gradient as if gradientUnits has not been specified)
    else if(gradientUnits != "objectBoundingBox") {
    }

    // spreadMethod attribute, default is "pad" set above
    QString spreadMethod(getTrait(element, "spreadMethod"));
    if(spreadMethod == "reflect") {
	g.setSpread(QGradient::ReflectSpread);
    }
    else if(spreadMethod == "repeat") {
	g.setSpread(QGradient::RepeatSpread);
    }
    // TODO: does spec say to use default value or referenced gradient's value
    // (I believe it would be referenced gradient as if spreadMethod has not been specified)
    else if(spreadMethod != "pad") {
    }

    // if this gradient had stops, we use them, otherwise we use the referenced gradient's
    QGradientStops stops = fetchGradientStops(element, opacity);
    if(stops.size() > 0) {
	g.setStops(stops);
    }

    return g;
}

QRadialGradient resolveRadialGradient(const QDomElement& element, qreal opacity, QStack<QString> referenceStack) {
    // default start/stop for SVG is cx=(0.5,0.5) r=0.5, fx=cx
    QRadialGradient g(0.5,0.5,0.5,0.5,0.5);
    // default userSpaceOnUse="objectBoundingBox"
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    // default spreadMethod="pad"
    g.setSpread(QGradient::PadSpread);

    // check the xlink:href attribute on this element, if it refers to a valid linear/radial gradient
    // that is not in our referenceStack, then go fetch it and copy the valid attributes and stops
    bool bOk = false;
    QString href = getTrait(element, "xlink:href", &bOk);
    if(bOk && !href.isEmpty()) {
	int refIndex = uriFrag.indexIn(href);
	if(refIndex == 0) {
	    QString refID = href.mid(refIndex+1);
	    if(!referenceStack.contains(refID)) {
		// fetch the new element
		QDomElement refElem = getElementById(element.ownerDocument().documentElement(), refID);
		if(!refElem.isNull()) {
		    // determine if it's a linear/radial gradient
		    QString tagName(refElem.tagName());
		    if(tagName == "linearGradient") {
			referenceStack.push(refID);
			// recursively call the resolve function to get the gradient
			QLinearGradient refGrad = resolveLinearGradient(refElem, opacity, referenceStack);
			// apply relevant stops and attributes to g
			g.setCoordinateMode(refGrad.coordinateMode());
			g.setSpread(refGrad.spread());
			g.setStops(refGrad.stops());
		    }
		    else if(tagName == "radialGradient") {
			referenceStack.push(refID);
			// recursively call the resolve function to get the gradient
			QRadialGradient refGrad = resolveRadialGradient(refElem, opacity, referenceStack);
			// apply relevant stops and attributes to g
			g.setCenter(refGrad.center());
			g.setRadius(refGrad.radius());
			g.setFocalPoint(refGrad.focalPoint());
			g.setCoordinateMode(refGrad.coordinateMode());
			g.setSpread(refGrad.spread());
		    }
		    else {
			cout << "Error! Reference '" << refID.toStdString().c_str() << "' is not a linear/radial gradient" << endl;
		    }
		}
		else {
		    cout << "Error!  Gradient reference '" << refID.toStdString().c_str() << "' not found" << endl;
		}
	    }
	    else {
		cout << "Error!  Circular reference detected with gradient '" << refID.toStdString().c_str() << "'" << endl;
	    }
	} // found a URI frag
    } // found xlink:href

    // now fetch the rest of this element's attributes and override any referenced gradient above

    // this will get the linear gradient's current x1,y1 and x2,y2
    // (which may be set by a referenced gradient)
    QPointF curCenter = g.center();
    qreal curRad = g.radius();
    QPointF curFocus = g.focalPoint();

    // TODO: handle percentages here
    bool bcx = false, bcy = false, br = false, bfx = false, bfy = false;
    qreal cx = element.attribute("cx", QString("%1").arg(curCenter.x())).toDouble(&bcx);
    qreal cy = element.attribute("cy", QString("%1").arg(curCenter.y())).toDouble(&bcx);
    qreal r = element.attribute("r", QString("%1").arg(curRad)).toDouble(&br);
    qreal fx = element.attribute("fx", QString("%1").arg(curFocus.x())).toDouble(&bfx);
    qreal fy = element.attribute("fy", QString("%1").arg(curFocus.y())).toDouble(&bfy);

    if(!bcx) { cx = curCenter.x(); }
    if(!bcy) { cy = curCenter.y(); }
    if(!br) { r = curRad; }
    if(!bfx) { fx = curFocus.x(); }
    if(!bfy) { fy = curFocus.y(); }

    g.setCenter(cx, cy);
    g.setRadius(r);
    g.setFocalPoint(fx, fy);

    // gradientUnits attribute
    // (gradientUnits="userSpaceOnUse", objectBoundingBox is the default mode set above)
    QString gradientUnits(getTrait(element, "gradientUnits"));
    if(gradientUnits == "userSpaceOnUse") {
	g.setCoordinateMode(QGradient::LogicalMode);
    }
    // TODO: does spec say to use default value or referenced gradient's value
    // (I believe it would be referenced gradient as if gradientUnits has not been specified)
    else if(gradientUnits != "objectBoundingBox") {
    }

    // spreadMethod attribute, default is "pad" set above
    QString spreadMethod(getTrait(element, "spreadMethod"));
    if(spreadMethod == "reflect") {
	g.setSpread(QGradient::ReflectSpread);
    }
    else if(spreadMethod == "repeat") {
	g.setSpread(QGradient::RepeatSpread);
    }
    // TODO: does spec say to use default value or referenced gradient's value
    // (I believe it would be referenced gradient as if spreadMethod has not been specified)
    else if(spreadMethod != "pad") {
    }

    // if this gradient had stops, we use them, otherwise we use the referenced gradient's
    QGradientStops stops = fetchGradientStops(element, opacity);
    if(stops.size() > 0) {
	g.setStops(stops);
    }

    return g;
}

Qt::AspectRatioMode getAspectRatio(const QDomElement& element) {
    // parse preserveAspectRatio attribute only if viewBox has been provided
    // preserveAspectRatio="[defer] <align>  [<meetOrSlice>]"
    // we ignore the 'defer' portion for now (<image> allows it)
    QRegExp alignRE("[xYMindaxoe]{4,}");
    QRegExp sliceRE("\\s+slice\\s*");
    QString par = getTrait(element, "preserveAspectRatio");
    int alignIndex = alignRE.indexIn(par);
    if(!par.isEmpty() && alignIndex >= 0) {
	int alignLength = alignRE.matchedLength();
	QString align(par.mid(alignIndex, alignLength));

	if(align == "none") { return Qt::IgnoreAspectRatio; }

	/*
	if(align == "none") { this->preserveAspectRatio_ = none; }
	else if(align == "xMinYMin") { this->preserveAspectRatio_ = xMinYMin; }
	else if(align == "xMidYMin") { this->preserveAspectRatio_ = xMidYMin; }
	else if(align == "xMaxYMin") { this->preserveAspectRatio_ = xMaxYMin; }
	else if(align == "xMinYMid") { this->preserveAspectRatio_ = xMinYMid; }
	else if(align == "xMaxYMid") { this->preserveAspectRatio_ = xMaxYMid; }
	else if(align == "xMinYMax") { this->preserveAspectRatio_ = xMinYMax; }
	else if(align == "xMidYMax") { this->preserveAspectRatio_ = xMidYMax; }
	else if(align == "xMaxYMax") { this->preserveAspectRatio_ = xMaxYMax; }
	else { this->preserveAspectRatio_ = xMidYMid; } // lacuna value
	*/

	int mosIndex = sliceRE.indexIn(par, alignIndex+alignLength);
	if(mosIndex > alignIndex) {
	    return Qt::KeepAspectRatioByExpanding; // "slice"
	}
    }

    return Qt::KeepAspectRatio; // "meet", the default
}
