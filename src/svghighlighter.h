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
#ifndef SVGHIGHLIGHTER_H
#define SVGHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class SVGHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SVGHighlighter(QTextDocument* parent = 0);

    void highlightBlock(const QString &text);

private:
    QStringList svgTags;
    QStringList svgAttrs;

    QTextCharFormat svgElemFormat;
    QTextCharFormat svgAttrFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat textContentsFormat;
    QTextCharFormat cdataFormat;
    QTextCharFormat malformedFormat;

};

#endif // SVGHIGHLIGHTER_H
