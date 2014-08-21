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
#ifndef CARVESVGDOCUMENT_H
#define CARVESVGDOCUMENT_H

#include <QString>
#include <QDomDocument>
#include <QAbstractItemModel>

class CarveSVGNode;
class CarveSVGWindow;
class CarveSVGElement;

// TODO: make doc_ a pointer?

class CarveSVGDocument : public QAbstractItemModel
{
    Q_OBJECT

public:
    CarveSVGDocument(const QString& textContent, CarveSVGWindow* parent);
    ~CarveSVGDocument();

    bool setContent(const QString& text);
    CarveSVGNode* root() { return root_; }
    QDomDocument* domDocument() { return &doc_; }

    // QAbstractItemModel interface
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& child) const;

    CarveSVGWindow* window() { return window_; }

    CarveSVGElement* svgElem();

private:
    // unimplemented to prevent copying
    CarveSVGDocument& operator=(const CarveSVGDocument&);
    CarveSVGDocument(const CarveSVGDocument&);

    QDomDocument doc_;
    CarveSVGNode* root_;
    CarveSVGWindow* window_;
};

#endif // CARVESVGDOCUMENT_H
