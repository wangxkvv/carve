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
#include "carvepreviewwindow.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QGraphicsSvgItem>

#include <iostream>
using std::cout;
using std::endl;

CarvePreviewWindow::CarvePreviewWindow(QWidget* parent, const QString& filename) :
        QDialog(parent, Qt::Window | Qt::Dialog | Qt::Popup), view(0)
{
    view = new QGraphicsView(this);
    view->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    view->setScene(new QGraphicsScene(view));

    QGraphicsSvgItem* svgItem = new QGraphicsSvgItem(filename);
    svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
    svgItem->setCacheMode(QGraphicsItem::NoCache);
    svgItem->setZValue(0);
    view->scene()->addItem(svgItem);

    QGridLayout* layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(0, 1);
    layout->addWidget(view, 0, 0);
    this->setLayout(layout);

    this->setWindowTitle("Preview Window");
}
