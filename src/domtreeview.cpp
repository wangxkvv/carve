#include "domtreeview.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QTextBlock>
#include "carvewindow.h"
#include "carvesvgnode.h"
#include "carvesvgwindow.h"
#include "carvesvgdocument.h"

#include <iostream>
using std::cout;
using std::endl;

DomTreeView::DomTreeView(CarveWindow* window, QSize hint) :
	QTreeView(), hint_(hint), mainwindow(window)
{
    setIndentation(10);

    menuContext = new QMenu(this);
    actionDeleteNode = new QAction("Delete Node", menuContext);
    menuContext->addAction(actionDeleteNode);

    // ensures that a right-mouse click will select the item under the mouse
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(nodeClicked(const QModelIndex&)));
}

// TODO: figure out the right way initiate a re-serialization of the DOM (shake out the API)
void DomTreeView::contextMenuEvent(QContextMenuEvent* e)
{
    QTreeView::contextMenuEvent(e);

    // deselect selection if not on it when clicked
    if(!this->indexAt(e->pos()).isValid()) {
	this->clearSelection();
    }

    QModelIndexList indices = this->selectedIndexes();
    if(indices.length() > 0) {
	QModelIndex index = indices.at(0);
	if(!index.isValid()) { return; }

	CarveSVGNode* node = static_cast<CarveSVGNode*>(index.internalPointer());
	mainwindow->selectNode(node);

	menuContext->move(e->globalPos());
	QAction* selectedAction = menuContext->exec();
	if(selectedAction == actionDeleteNode) {
	    mainwindow->deleteNode(node);
	}
    }
}

void DomTreeView::setWindow(CarveSVGWindow* w) {
    if(!w) { return; }
    this->window = w;
    this->setModel(this->window->model());
	this->setRootIsDecorated(true);
}

void DomTreeView::nodeClicked(const QModelIndex& index) {
    CarveSVGNode* node = static_cast<CarveSVGNode*>(index.internalPointer());
    mainwindow->selectNode(node);
}

bool DomTreeView::findIndex(CarveSVGNode* node, QModelIndex& index) {
    bool bFound = false;
    if(!index.isValid()) {
	return false;
    }

    CarveSVGNode* iNode = static_cast<CarveSVGNode*>(index.internalPointer());
    if(node == iNode) { bFound = true; }

    // if the current node is not the node we're searching for, search the children
    if(!bFound) {
	QModelIndex saveIndex = index;
	int numChildren = this->model()->rowCount(index);
	for(int i = 0; i < numChildren; ++i) {
	    QModelIndex childIndex = index.child(i,0);
	    if(findIndex(node, childIndex)) {
		index = childIndex;
		return true;
	    }
	}
    }
    return bFound;
}

#include <QModelIndexList>

void DomTreeView::nodeSelected(CarveSVGNode* node) {
    if(!node) {
	this->clearSelection();
	return;
    }

    QModelIndex modelIndex = this->model()->index(0, 0, QModelIndex());
    if(this->findIndex(node, modelIndex)) {

	QModelIndexList indices(this->selectedIndexes());
	if(indices.length() > 0) {
	    // loop for each selected index
	    for(int i = 0; i < indices.length(); ++i) {
		// if that index was already selected, just return
		if(indices.at(i) == modelIndex) {
//		    cout << "Index was already selected" << endl;
		    return;
		}
	    }
	}

	this->setCurrentIndex(modelIndex);
    }

    // TODO: this should be moved to the CarevSVGWindow and handled there as part of the nodeSelected() signal catching
    QDomElement domElem = node->domElem();
    QTextCursor cursor(this->window->edit()->document()->findBlockByLineNumber(domElem.lineNumber()));
    this->window->edit()->setTextCursor(cursor);
    this->window->edit()->ensureCursorVisible();

    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection extra;
    extra.format.setBackground(QColor(255,255,210));
    extra.format.setProperty(QTextFormat::FullWidthSelection, true);
    extra.cursor = this->window->edit()->textCursor();
    extra.cursor.clearSelection();
    selections.append(extra);
    this->window->edit()->setExtraSelections(selections);
}
