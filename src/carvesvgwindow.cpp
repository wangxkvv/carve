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
#include "carvesvgwindow.h"
#include "carvesvgdocument.h"
#include "svghighlighter.h"
#include "carvesvgnode.h"
#include "carvescene.h"
#include "carvedesignview.h"
#include "version.h"
#include "carvewindow.h"
#include "carvesvgelement.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileDialog>
#include <QCloseEvent>
#include <QXmlStreamReader>
#include <QStack>
#include <QPlainTextEdit>
#include <QGridLayout>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

CarveSVGWindow::CarveSVGWindow(CarveWindow* window) : QStackedWidget(),
    untitled_(true),
    filename_(""),
    mode_(Code), mainwindow_(window)
{

    static int numUntitledFiles = 1;
    if(numUntitledFiles > 1) {
	setWindowTitle(QString("Untitled SVG %1 [*]").arg(numUntitledFiles));
    }
    else {
	setWindowTitle("Untitled SVG [*]");
    }
    ++numUntitledFiles;

    // Plain text edit is the first widget in the stack
    this->edit_ = new QPlainTextEdit();
    this->addWidget(this->edit_);

    // constructing a highlighter with the document as an argument is all we need to do
    // (this feels backward here, but that's Qt's fault)
    new SVGHighlighter(this->edit_->document());

    // Graphics view is the second widget in the stack
    this->scene_ = new CarveScene(mainwindow_);
    this->view_ = new CarveDesignView(this);
    this->view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    this->view_->setScene(this->scene_);
    this->addWidget(this->view_);

    // TODO: is <?xml...?> required here?  better to author without it?  AI seems to trip and it gets into the DOM Browser...
    QString StarterDoc = QString("<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'\n     version='1.2' baseProfile='tiny'>\n"
		     "  <!-- Created with Carve %1 (%2) -->\n"
                     "</svg>").arg(SW_CODENAME).arg(SW_VERSION);

    // by making the window the model's parent, I believe this will take
    // care of destroying the model at the appropriate time
    model_ = new CarveSVGDocument(StarterDoc, this);

    // seed our text area with initial SVG bare-bones
    this->edit_->setPlainText(StarterDoc);
    this->setWindowModified(true);

    this->edit_->document()->setModified(true);

    init();
}

bool CarveSVGWindow::load(const QString& filename) {
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Carve"),
            tr("Cannot read file %1:\n%2.")
            .arg(filename)
            .arg(file.errorString()));
        return false;
    }

    filename_ = filename;
    untitled_ = false;

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setWindowTitle(QFileInfo(filename_).fileName() + "[*]");

    // TDOO: make this a separate function that we call in constructor, load and any time the content changes
    // now set the text of the document, DOM document and determine the character positions
    // of each node in the document
    this->scene_->clear();
    QString rawText = in.readAll();
    this->edit_->setPlainText(rawText);
    this->model_->setContent(rawText);
    // ========================================================================

    QApplication::restoreOverrideCursor();
    this->setWindowModified(false);

    init();
    return true;
}

bool CarveSVGWindow::save(const QString& lastPath) {
    if(isUntitled()) {
        return saveAs(lastPath);
    }
    return saveFile();
}

bool CarveSVGWindow::saveAs(const QString& lastPath) {
    QString path = lastPath; path.append("/").append(filename_);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), path, "SVG Files (*.svg)");
    if (fileName.isEmpty())
        return false;

    filename_ = fileName;
    dir_ = QFileInfo(filename_).absoluteDir();
//    cout << "filename='" << filename_.toStdString().c_str() << "'" << endl;
//    cout << "dir='" << dir_.absolutePath().toStdString().c_str() << "'" << endl;
//    cout << "path='" << path.toStdString().c_str() << "'" << endl;
    setWindowTitle(QFileInfo(filename_).fileName());
    untitled_ = false;

    return saveFile();
}

bool CarveSVGWindow::saveFile() {
    // filename_ now contains the file we want to save
    QFile file(filename_);
    if(!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
            tr("Cannot write file %1:\n%2.")
            .arg(filename_)
            .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << this->edit_->toPlainText();
    QApplication::restoreOverrideCursor();
    this->edit_->document()->setModified(false);
    this->setWindowModified(false);

    return true;
}

void CarveSVGWindow::closeEvent(QCloseEvent* event) {
    if(edit_->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("MDI"),
            tr("'%1' has been modified.\nDo you want to save your changes?")
            .arg(this->windowTitle()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            if(save(dir_.absolutePath())) { event->accept(); }
            else { event->ignore(); }
        }
        else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    else {
	delete this->scene_;
        event->accept();
    }
}

void CarveSVGWindow::init() {
    connect(this->edit_->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
}

void CarveSVGWindow::documentWasModified() {
    this->setWindowModified(this->edit_->document()->isModified());
}

bool CarveSVGWindow::isValidXML() {
    QString t = this->edit_->toPlainText();
    delete this->scene_;
    this->scene_ = new CarveScene(this->mainwindow_);
    this->view_->resetMatrix();
    this->view_->setScene(this->scene_);
    bool bResult = model_->setContent(t);
    CarveSVGElement* svg = model_->svgElem();
    if(svg) {
	svg->update(view_->width(), view_->height());
    }
    else {
	bResult = false;
    }
    return bResult;
}

CarveSVGDocument* CarveSVGWindow::model() {
    return model_;
}

void CarveSVGWindow::switchMode(SVGWindowMode newMode) {
    mode_ = newMode;
    if(newMode == Code) {
	this->setCurrentWidget( this->edit_ );
    }
    else {
	this->setCurrentWidget( this->view_ );
    }
}

void CarveSVGWindow::updateDocImmediately() {
    this->mainwindow_->updateDocImmediately();
}
