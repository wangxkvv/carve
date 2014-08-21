#include "carvescene.h"
#include "carvewindow.h"
#include "carvesvgnode.h"

#include <QGraphicsItem>

#include <iostream>
using std::cout;
using std::endl;

CarveScene::CarveScene(CarveWindow* window) :
	QGraphicsScene(),
	mainwindow_(window)
{
    setBackgroundBrush(QBrush(QColor(224,224,224)));

    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectNode()));
    connect(mainwindow_, SIGNAL(nodeSelected(CarveSVGNode*)), this, SLOT(nodeSelected(CarveSVGNode*)));
    connect(mainwindow_, SIGNAL(nodeDeleted(CarveSVGNode*)), this, SLOT(nodeDeleted(CarveSVGNode*)));
}

void CarveScene::selectNode() {
    QList<QGraphicsItem*> selected(this->selectedItems());
    if(selected.length() == 1) {
	QGraphicsItem* item = selected.at(0);
	if(!item) { return; }
	CarveSVGNode* node = reinterpret_cast<CarveSVGNode*>(item->data(0).value<void*>());
	if(!node) { return; }
	mainwindow_->selectNode(node);
    }
}

// this slot catches the nodeSelected() signal coming from main window
void CarveScene::nodeSelected(CarveSVGNode* node) {
    if(!node || !node->gfxItem()) {
	this->clearSelection();
	return;
    }

    // if this gfx item is already selected, then do nothing
    QList<QGraphicsItem*> selected(this->selectedItems());
    for(int i = 0; i < selected.length(); ++i) {
	if(selected.at(i) == node->gfxItem()) { return; }
    }

    // else, select it programmatically
    this->clearSelection();
    node->gfxItem()->setSelected(true);
}

void CarveScene::nodeDeleted(CarveSVGNode* node) {
    if(!node || !node->gfxItem()) {
	return;
    }
    this->removeItem(node->gfxItem());
}
