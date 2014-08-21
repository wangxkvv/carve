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
#include "carvedesignview.h"
#include <QResizeEvent>
#include <QMatrix>
#include "carvesvgwindow.h"
#include "carvesvgdocument.h"
#include "carvesvgnode.h"
#include "carvesvgelement.h"

#include <QMenu>
#include <QAction>

#include <iostream>
using std::cout;
using std::endl;

CarveDesignView::CarveDesignView(CarveSVGWindow* window) : QGraphicsView(),
    window_(window)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // TODO: Do I need to delete this?
    contextMenu_ = new QMenu(this);
    deleteNode = new QAction(tr("Delete"), this);
    contextMenu_->addAction(deleteNode);
}

void CarveDesignView::resizeEvent(QResizeEvent* event) {
    // call the CarveSVGElement's update() with the new view width/height
    CarveSVGElement* svg = this->window_->model()->svgElem();
    if(svg) {
	svg->update(this->width(), this->height());
    }
    else {
	cout << "Error!  Could not get the <svg> element from the CarveSVGDocument" << endl;
    }

    // after this point, the scene should have the proper size so we can scale the view properly

    QGraphicsView::resizeEvent(event);
}
