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
  This encapsulates an SVG document including its:
  - model
  - DOM document
*/

#include "carvesvgdocument.h"
#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvesvgelement.h"
#include "carvescene.h"

#include <iostream>
using std::cout;
using std::endl;

CarveSVGDocument::CarveSVGDocument(const QString& textContent, CarveSVGWindow* parent) :
	QAbstractItemModel(parent), window_(parent)
{
    root_ = NULL;
    setContent(textContent);
}

CarveSVGDocument::~CarveSVGDocument() {
    delete root_;
}

bool CarveSVGDocument::setContent(const QString& text) {
    /*
    // wipe out scene
    if(this->window()->scene()) {
	this->window()->scene()->clear();
    }
    */

    // wipe out old model
    if(root_) delete root_;

    // set the QDomDocument's contents
    // TODO: use text.toUtf8() here?
    // we use namespace processing = false here to ensure that prefixes don't get munged around when re-serializing
    // (we want the namespace prefixes to be where we typed them)
    bool bResult = this->doc_.setContent(text, false);

    // set up new data model
//    QDomElement rootDomNode(doc_.documentElement());
    root_ = CarveSVGNode::createNode(doc_, 0, this->window_);

    // inform all views that we've reset
    reset();

    return bResult;
}


// QAbstractItemModel interface
// element type and id
int CarveSVGDocument::columnCount(const QModelIndex& ) const {
    return 2;
}

Qt::ItemFlags CarveSVGDocument::flags(const QModelIndex& index) const {
    if(!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CarveSVGDocument::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Element");
            case 1: return tr("id");
            default: return QVariant();
        }
    }

    return QVariant();
}

QModelIndex CarveSVGDocument::index(int row, int column, const QModelIndex& parent) const {
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    CarveSVGNode* parentItem;

    if(!parent.isValid()) {
        parentItem = this->root_;
    }
    else {
        parentItem = static_cast<CarveSVGNode*>(parent.internalPointer());
    }

    CarveSVGNode* childItem = parentItem->child(row);
    if(childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

int CarveSVGDocument::rowCount(const QModelIndex& parent) const {
    if(parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
	// there can only be 1 top-level node
	return 1;
    }

    CarveSVGNode* parentItem = static_cast<CarveSVGNode*>(parent.internalPointer());
    return parentItem->domElem().childNodes().count();
}

QModelIndex CarveSVGDocument::parent(const QModelIndex& child) const {
    if(!child.isValid()) {
        return QModelIndex();
    }

    CarveSVGNode* childItem = static_cast<CarveSVGNode*>(child.internalPointer());
    CarveSVGNode* parentItem = childItem->parent();

    if(!parentItem || parentItem == this->root_) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant CarveSVGDocument::data(const QModelIndex& index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    CarveSVGNode* item = static_cast<CarveSVGNode*>(index.internalPointer());

    QDomElement node = item->domElem();

    switch (index.column()) {
        // return the node type
        case 0: return node.nodeName();

        // return the id
        case 1: {
            QDomNamedNodeMap attrs = node.attributes();
            if(!attrs.contains("id")) { // && !attrs.contains("xml:id")) {
                return QString("");
            }

            QDomNode id = attrs.namedItem("id");
            return id.nodeValue();
        }

        default: return QVariant();
    }
}

CarveSVGElement* CarveSVGDocument::svgElem() {
    if(root()->numChildren() != 1) {
	cout << "Error!  Doc root did not have one child!" << endl;
    }
    else {
	CarveSVGNode* docNode = root()->child(0);
	if(!docNode || docNode->type() != svgSvg) {
	    cout << "Error!  Root of document was not of type <svg>" << endl;
	}
	else {
	    CarveSVGElement* svg = dynamic_cast<CarveSVGElement*>(docNode);
	    if(!svg) {
		cout << "Could not find the <svg> node" << endl;
	    }
	    else {
		return svg;
	    }
	}
    }
    return NULL;
}
