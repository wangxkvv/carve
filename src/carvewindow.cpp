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
#include "carvewindow.h"
#include "../ui_HelpDialog.h"
#include "../ui_PreferencesDialog.h"
#include "../ui_FindDialog.h"
#include "carvesvgdocument.h"
#include "carvesvgwindow.h"
#include "carvepreviewwindow.h"
#include "version.h"
#include "domtreeview.h"
#include "propertiespane.h"

#include <QtGlobal>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QCloseEvent>
#include <QMessageBox>
#include <QClipboard>
#include <QSvgWidget>
#include <QIcon>
#include <QTimer>
#include <QFontDialog>
#include <QTreeView>
#include <QDockWidget>
#include <QGraphicsView>
#include <QGraphicsSvgItem>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

const char* szXMLDisabled = "<span style='background-color:#eee; color:lightgrey; font-weight:bold'>&nbsp;XML&nbsp;</span>";
const char* szXMLValid = "<span style='background-color:green; color:white; font-weight:bold'>&nbsp;XML&nbsp;</span>";
const char* szXMLInvalid = "<span style='background-color:red; color:white; font-weight:bold'>&nbsp;XML&nbsp;</span>";

const int MAX_DOCUMENTS = 16;
const int REFRESH_XML_TIMER = 2000;

Ui::PreferencesDialog prefUI;

// TODO: make this factory method a separate compilation unit eventually

#include "carvesvgnode.h"
#include "carvesvgelement.h"
#include "carverectelement.h"
#include "carvecircleelement.h"
#include "carveellipseelement.h"
#include "carvelineelement.h"
#include "carvepolylineelement.h"
#include "carvepolygonelement.h"
#include "carvepathelement.h"
#include "carvegelement.h"
#include "carvetextelement.h"
#include "carveaelement.h"
#include "carveimageelement.h"

CarveSVGNode* CarveSVGNode::createNode(const QDomDocument& doc, int row, CarveSVGWindow* window, CarveSVGNode* parent) {
    return new CarveSVGNode(doc, row, window, svgUndefined, parent);
}

CarveSVGNode* CarveSVGNode::createNode(const QDomElement& node, int row, CarveSVGWindow* window, CarveSVGNode* parent) {
    QString tagName = node.nodeName();
    // subclasses of CarveSVGNode
    if(tagName == "svg") { return new CarveSVGElement(node, row, window, parent); }
    else if(tagName == "rect") { return new CarveRectElement(node, row, window, parent); }
    else if(tagName == "circle") { return new CarveCircleElement(node, row, window, parent); }
    else if(tagName == "ellipse") { return new CarveEllipseElement(node, row, window, parent); }
    else if(tagName == "line") { return new CarveLineElement(node, row, window, parent); }
    else if(tagName == "polyline") { return new CarvePolylineElement(node, row, window, parent); }
    else if(tagName == "polygon") { return new CarvePolygonElement(node, row, window, parent); }
    else if(tagName == "path") { return new CarvePathElement(node, row, window, parent); }
    else if(tagName == "g") { return new CarveGElement(node, row, window, parent); }
    else if(tagName == "text") { return new CarveTextElement(node, row, window, parent); }
    else if(tagName == "a") { return new CarveAElement(node, row, window, parent); }
    else if(tagName == "image") { return new CarveImageElement(node, row, window, parent); }
    // instances of CarveSVGNode
    else if(tagName == "linearGradient") { return new CarveSVGNode(node, row, window, svgLinearGradient, parent); }
    else if(tagName == "radialGradient") { return new CarveSVGNode(node, row, window, svgRadialGradient, parent); }
    else if(tagName == "stop") { return new CarveSVGNode(node, row, window, svgStop, parent); }
    else if(tagName == "defs") { return new CarveSVGNode(node, row, window, svgDefs, parent); }
    // unimplemented
    return new CarveSVGNode(node, row, window, svgUndefined, parent);
}

CarveWindow::CarveWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags),
      domBrowserColumn0Width(-1),
      timerDocModified(new QTimer(this)),
      lastFindText(""), lastReplaceText("")
{
    QIcon icon(":/toolbars/images/Carve-48.png");
    this->setWindowIcon(icon);

    ui.setupUi(this);

    prefDlg = new QDialog(this);
    prefUI.setupUi(prefDlg);

    // if Windows, turn Edit > Preferences into Tools > Options
    // NOTE: Mac automatically puts Edit > Preferences into the Carve menu
#ifdef Q_WS_WIN
    this->ui.menuEdit->removeAction(this->ui.actionPreferences);
    this->ui.actionPreferences->setText("Options");
    this->ui.menuTools->addAction(this->ui.actionPreferences);
#else
    // hide Tools menu for now on Unix-y systems
    this->ui.menuTools->menuAction()->setVisible(false);
#endif

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
	// set keyboard shortcut for recently opened file
	if(i < 9) {
	    QString shortcut = QString("Ctrl+%1").arg(i+1);
	    recentFileActs[i]->setShortcut(QKeySequence(shortcut));
	}
        ui.menuRecentFiles->addAction(recentFileActs[i]);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(fileOpenRecentFile()));
    }

    // set central widget of the main window to the MDI area
    this->setCentralWidget(ui.mdiArea);
    ui.mdiArea->setDocumentMode(true);
    ui.mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui.mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->setUnifiedTitleAndToolBarOnMac(true);

    // set up permanent statusbar widget for XML validity
    labelXML = new QLabel();
    labelXML->setTextFormat(Qt::RichText);
    labelXML->setText(szXMLDisabled);
    labelXML->setToolTip("Indicates whether the SVG document is valid XML.");
    this->statusBar()->insertPermanentWidget(0,labelXML);

    loadSettings();

    connect(ui.actionFileNew, SIGNAL(triggered()), this, SLOT(fileNew()));
    connect(ui.actionFileOpen, SIGNAL(triggered()), this, SLOT(fileOpen()));
    connect(ui.actionFileSave, SIGNAL(triggered()), this, SLOT(fileSave()));
    connect(ui.actionFileSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    connect(ui.actionExportAsPNG, SIGNAL(triggered()), this, SLOT(fileExportPNG()));
    connect(ui.actionFileClose, SIGNAL(triggered()), ui.mdiArea, SLOT(closeActiveSubWindow()));
    connect(ui.actionFileCloseAll, SIGNAL(triggered()), ui.mdiArea, SLOT(closeAllSubWindows()));
    connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(editUndo()));
    connect(ui.actionRedo, SIGNAL(triggered()), this, SLOT(editRedo()));
    connect(ui.actionCopy, SIGNAL(triggered()), this, SLOT(editCopy()));
    connect(ui.actionCut, SIGNAL(triggered()), this, SLOT(editCut()));
    connect(ui.actionPaste, SIGNAL(triggered()), this, SLOT(editPaste()));
    connect(ui.actionPlay, SIGNAL(triggered()), this, SLOT(controlPlay()));
    connect(ui.actionFind, SIGNAL(triggered()), this, SLOT(editFind()));
    connect(ui.actionFindNext, SIGNAL(triggered()), this, SLOT(editFindNext()));
    connect(ui.actionFindPrev, SIGNAL(triggered()), this, SLOT(editFindPrev()));
    connect(ui.actionReplace, SIGNAL(triggered()), this, SLOT(editReplace()));
    connect(ui.actionReplaceNext, SIGNAL(triggered()), this, SLOT(editReplaceNext()));
    connect(ui.actionQuit_Carve, SIGNAL(triggered()), this, SLOT(fileQuit()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(preferences()));
    connect(ui.actionAboutCarve, SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(ui.mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateWindowMenu(QMdiSubWindow*)));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(enablePaste()));
    connect(timerDocModified, SIGNAL(timeout()), this, SLOT(refreshXMLStatus()));
    connect(prefUI.changeFontButton, SIGNAL(clicked()), this, SLOT(chooseNewEditorFont()));
    connect(ui.actionCode, SIGNAL(triggered()), this, SLOT(modeCodeClicked()));
    connect(ui.actionDesign, SIGNAL(triggered()), this, SLOT(modeDesignClicked()));
    connect(ui.actionSwitchMode, SIGNAL(triggered()), this, SLOT(switchMode()));
    connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    ui.actionUndo->setDisabled(true);
    ui.actionRedo->setDisabled(true);
    ui.actionCut->setDisabled(true);
    ui.actionCopy->setDisabled(true);
    ui.actionPaste->setDisabled(true);
    ui.actionFindNext->setDisabled(true);
    ui.actionFindPrev->setDisabled(true);
    ui.actionReplaceNext->setDisabled(true);

    updateDocumentMenus();

    this->timerDocModified->start(REFRESH_XML_TIMER);
}

void CarveWindow::closeEvent(QCloseEvent* event) {
    // save off each document (or pop up a warning)
    ui.mdiArea->closeAllSubWindows();

    // after attempting to close all sub-windows, if there's any window open, then user cancelled the Close
    if(activeSVGWindow()) {
        event->ignore();
    } else {
        saveSettings();
        event->accept();
    }
}

void CarveWindow::loadSettings() {
    QSettings settings("codedread", "Carve");

    // last loaded/saved path and recent files
    lastPath = settings.value("lastPath",
#if defined(Q_WS_WIN)
                              QString("%%HOMEDRIVE%%%%HOMEPATH%%\\My Documents\\")
#elif defined(Q_WS_MAC)
                              QDir::homePath().append("/Documents/")
#else
                              QDir::homePath()
#endif
                              ).toString();
    updateRecentFileActions();
    // ========================================================================

    settings.beginGroup("window");
    bool bMaximized = settings.value("maximized", false).toBool();

    // main window state
    windowPos = settings.value("pos", QPoint(0,0)).toPoint();
    windowSize = settings.value("size", QSize(600,400)).toSize();
    if(bMaximized) {
        this->showMaximized();
    }
    else {
        this->move(windowPos);
        this->resize(windowSize);
    }
    settings.endGroup();
    // ========================================================================

    // DOM browser pane state
    settings.beginGroup("dombrowser");
    bool bDOMBrowserShown = settings.value("visible", true).toBool();
    // 0=left, 1=right, 2=floating
    int domBrowserState = settings.value("state", 0).toInt();
    QSize domBrowserSize = settings.value("size", QSize(256,256)).toSize();
    QPoint domBrowserPos = settings.value("pos", QPoint(200,200)).toPoint();
    // set up dockable DOM browser pane
    domTree = new DomTreeView(this, domBrowserSize);
    // ensure if the column's width was ever shrunk too much that we restore it a bit
    domBrowserColumn0Width = settings.value("column0.width",100).toInt();
    if(domBrowserColumn0Width < 10) { domBrowserColumn0Width = 10; }
    QDockWidget* domDock = new QDockWidget(tr("DOM Browser"), this);
    domDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    domDock->setWidget(domTree);
    ui.menuWindow->insertAction(ui.menuWindow->actions().at(0), domDock->toggleViewAction());
    connect(domDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(domBrowserDocked(Qt::DockWidgetArea)));
    connect(domTree->header(), SIGNAL(sectionResized(int,int,int)), this, SLOT(domBrowserHeaderChanged(int,int,int)));
    connect(this, SIGNAL(nodeSelected(CarveSVGNode*)), domTree, SLOT(nodeSelected(CarveSVGNode*)));
//    connect();
    domDock->toggleViewAction()->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));

    addDockWidget( domBrowserState == 1 ? Qt::RightDockWidgetArea : Qt::LeftDockWidgetArea, domDock);

    bDOMBrowserDockedLeft = (domBrowserState != 1);
    if(domBrowserState == 2) { // float
        domDock->setFloating(true);
    }
    if(!bDOMBrowserShown) {
        domDock->setVisible(false);
    }
    domDock->move(domBrowserPos);
    settings.endGroup();
    // ========================================================================

    // set up dockable Properties pane
    // Properties pane state
    settings.beginGroup("propertiespane");
    bool bPropPaneShow = settings.value("visible", true).toBool();
    Qt::DockWidgetArea propPaneState = (Qt::DockWidgetArea)settings.value("state", Qt::BottomDockWidgetArea).toInt();
    QSize propPaneSize = settings.value("size", QSize(256,256)).toSize();
    QPoint propPanePos = settings.value("pos", QPoint(200,200)).toPoint();

    QDockWidget* propDock = new QDockWidget(tr("Properties"), this);

    this->propPane = new PropertiesPane(propPaneSize);
    connect(this, SIGNAL(nodeSelected(CarveSVGNode*)), this->propPane, SLOT(setNode(CarveSVGNode*)));

    propDock->setWidget(this->propPane);

    ui.menuWindow->insertAction(ui.menuWindow->actions().at(1), propDock->toggleViewAction());
    propDock->toggleViewAction()->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    addDockWidget(propPaneState != Qt::NoDockWidgetArea ? propPaneState : Qt::BottomDockWidgetArea, propDock);
    if(propPaneState == Qt::NoDockWidgetArea) {
	propDock->setFloating(true);
    }

    if(!bPropPaneShow) {
	propDock->setVisible(false);
    }
    propDock->move(propPanePos);
    settings.endGroup();
    // ========================================================================

    // text editor font family and size
    settings.beginGroup("texteditor");
    textEditorNewFont.setFamily(settings.value("fontfamily", "Courier").toString());
    textEditorNewFont.setPointSize(settings.value("fontsize", 10).toInt());
    settings.endGroup();
    // ========================================================================
}

void CarveWindow::saveSettings() {
    QSettings settings("codedread", "Carve");

    settings.setValue("lastPath", this->lastPath);
    // ========================================================================

    // main window state
    settings.beginGroup("window");
    settings.setValue("maximized", this->isMaximized());
    this->windowPos = this->pos();
    this->windowSize = this->size();
    settings.setValue("pos", this->windowPos);
    settings.setValue("size", this->windowSize);
    settings.endGroup();
    // ========================================================================

    // DOM browser pane state
    settings.beginGroup("dombrowser");
    QDockWidget* domDock = qobject_cast<QDockWidget*>(this->domTree->parentWidget());
    settings.setValue("visible", domDock->isVisible());
    int domBrowserState = this->bDOMBrowserDockedLeft ? 0 : 1; // left/right
    if(domDock->isFloating()) { domBrowserState = 2; } // floating
    settings.setValue("state", domBrowserState);
    settings.setValue("size", QSize(domTree->size()));
    settings.setValue("pos", QPoint(domDock->pos()));
    if(this->domBrowserColumn0Width != -1) {
	settings.setValue("column0.width", domBrowserColumn0Width);
    }
    else { // does this ever get run?
	settings.setValue("column0.width", domTree->columnWidth(0));
    }
    settings.endGroup();
    // ========================================================================

    // Properties pane state
    settings.beginGroup("propertiespane");
    QDockWidget* propDock = qobject_cast<QDockWidget*>(propPane->parentWidget());
    settings.setValue("visible", propDock->isVisible());
    settings.setValue("size", QSize(propPane->size()));
    settings.setValue("pos", QPoint(propDock->pos()));
    Qt::DockWidgetArea propPaneState = this->dockWidgetArea(propDock);
    if(propDock->isFloating()) { propPaneState = Qt::NoDockWidgetArea; } // floating
    settings.setValue("state", (int)propPaneState);
    settings.endGroup();
    // ========================================================================

    // text editor font family and size
    settings.beginGroup("texteditor");
    settings.setValue("fontfamily", this->textEditorNewFont.family());
    settings.setValue("fontsize", this->textEditorNewFont.pointSize());
    settings.endGroup();
    // ========================================================================
}

CarveSVGWindow* CarveWindow::activeSVGWindow()
{
    if(QMdiSubWindow* activeSubWindow = ui.mdiArea->activeSubWindow()) {
        return qobject_cast<CarveSVGWindow*>(activeSubWindow->widget());
    }
    return 0;
}

QMdiSubWindow* CarveWindow::findChildWindow(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach(QMdiSubWindow *window, ui.mdiArea->subWindowList()) {
        CarveSVGWindow* mdiChild = qobject_cast<CarveSVGWindow*>(window->widget());
        if(mdiChild->getFilename() == canonicalFilePath)
            return window;
    }
    return 0;
}

CarveSVGWindow* CarveWindow::createMDIChild() {
    CarveSVGWindow* childWin = new CarveSVGWindow(this);
    this->ui.mdiArea->addSubWindow(childWin);
    connect(childWin->edit(), SIGNAL(destroyed()), this, SLOT(updateDocumentMenus()));
    connect(childWin->edit(), SIGNAL(copyAvailable(bool)), this->ui.actionCut, SLOT(setEnabled(bool)));
    connect(childWin->edit(), SIGNAL(copyAvailable(bool)), this->ui.actionCopy, SLOT(setEnabled(bool)));
    return childWin;
}

void CarveWindow::prepareNewChildWindow(CarveSVGWindow* childWin) {
    connect(childWin->edit()->document(), SIGNAL(contentsChanged()), this, SLOT(activeDocumentWasModified()));
    connect(childWin->edit(), SIGNAL(undoAvailable(bool)), ui.actionUndo, SLOT(setEnabled(bool)));
    connect(childWin->edit(), SIGNAL(redoAvailable(bool)), ui.actionRedo, SLOT(setEnabled(bool)));
    this->timerDocModified->start(0);
    childWin->setFont(this->textEditorNewFont);
    childWin->show();
}

void CarveWindow::fileNew() {
    // TODO: pop up some dialog to let them choose the filename or document type or something?

    // only allow N child windows at a time
    if(ui.mdiArea->subWindowList().size() < MAX_DOCUMENTS) {
        CarveSVGWindow* childWin = createMDIChild();
        statusBar()->showMessage(tr("New SVG document created"), 3000);
        prepareNewChildWindow(childWin);
    }
    else {
        QMessageBox::warning(this, "Carve", tr("Too many documents open.  Please close some documents"));
    }
}

void CarveWindow::fileOpen() {
    // only allow N child windows at a time
    if(ui.mdiArea->subWindowList().size() < MAX_DOCUMENTS) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open SVG Document"), lastPath, "SVG Documents (*.svg)");
        if (!fileName.isEmpty()) {
            // sync up lastPath
            lastPath = QFileInfo(fileName).absoluteDir().canonicalPath();

            QMdiSubWindow* existing = findChildWindow(fileName);
            if(existing) {
                ui.mdiArea->setActiveSubWindow(existing);
                return;
            }

            CarveSVGWindow *childWin = createMDIChild();
            if (childWin->load(fileName)) {
                prepareNewChildWindow(childWin);
                setCurrentFile(fileName);
                statusBar()->showMessage(tr("File loaded"), 3000);
            } else {
                childWin->close();
            }
        }
    }
    else {
        QMessageBox::warning(this, "Carve", tr("Too many documents open.  Please close some documents"));
    }
}

void CarveWindow::fileOpenRecentFile() {
     QAction *action = qobject_cast<QAction *>(sender());
     if(!action) return;

    // only allow N child windows at a time
    if(ui.mdiArea->subWindowList().size() < MAX_DOCUMENTS) {
        QString fileName = action->data().toString();
        if(!fileName.isEmpty()) {
            // do not sync up lastPath
//            lastPath = QFileInfo(fileName).absoluteDir().canonicalPath();

            QMdiSubWindow* existing = findChildWindow(fileName);
            if(existing) {
                ui.mdiArea->setActiveSubWindow(existing);
                return;
            }

            CarveSVGWindow *childWin = createMDIChild();
            if(childWin->load(fileName)) {
                prepareNewChildWindow(childWin);
                setCurrentFile(fileName);
                statusBar()->showMessage(tr("File loaded"), 3000);
            } else {
                childWin->close();
            }
        }
    }
    else {
        QMessageBox::warning(this, "Carve", tr("Too many documents open.  Please close some documents"));
    }
}

void CarveWindow::fileSave() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin) {
	if(childWin->isUntitled()) { fileSaveAs(); }
	else if(childWin->save(lastPath)) {
	    lastPath = QFileInfo(childWin->getFilename()).absoluteDir().canonicalPath().toStdString().c_str();
	    statusBar()->showMessage(tr("File saved"), 3000);
	}
    }
    else {
        // TODO: display some error dialog
    }
}

void CarveWindow::fileSaveAs() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin && childWin->saveAs(lastPath)) {
        lastPath = QFileInfo(childWin->getFilename()).absoluteDir().canonicalPath().toStdString().c_str();
        setCurrentFile(childWin->getFilename());
        statusBar()->showMessage(tr("File saved"), 3000);
    }
    else {
        // TODO: display some error dialog
    }
}

void CarveWindow::fileQuit() {
    this->close();
}

void CarveWindow::editUndo() {
    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	childWin->edit()->undo();
    }
}

void CarveWindow::editRedo() {
    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	childWin->edit()->redo();
    }
}

void CarveWindow::editCut() {
    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	childWin->edit()->cut();
    }
}

void CarveWindow::editCopy() {
    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	childWin->edit()->copy();
    }
}

void CarveWindow::editPaste() {
    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	childWin->edit()->paste();
    }
}

void CarveWindow::editFind() {
    if(!this->activeSVGWindow()) { return; }

    QDialog findDlg;
    Ui::FindDialog fui;
    fui.setupUi(&findDlg);
    fui.findTextEdit->setText(this->lastFindText);
    fui.findTextEdit->selectAll();
    fui.findTextEdit->setFocus();
    fui.replaceLabel->setDisabled(true);
    fui.replaceTextEdit->setDisabled(true);
    findDlg.setWindowTitle("Find Text");

    if(findDlg.exec() == QDialog::Accepted) {
        this->lastFindText = fui.findTextEdit->text();
        if(this->lastFindText.length() > 0) {
            this->ui.actionFindNext->setEnabled(true);
            this->ui.actionFindPrev->setEnabled(true);
	    this->ui.actionReplaceNext->setEnabled(true);
        }
	else {
	    this->ui.actionFindNext->setEnabled(false);
	    this->ui.actionFindPrev->setEnabled(false);
	    this->ui.actionReplaceNext->setEnabled(false);
	}

        CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
        if(childWin) {
	    QPlainTextEdit* edit = childWin->edit();
	    bool bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
	    if(!bFound) {
		// if cursor is not at beginning of document:
		QTextCursor originalCursor = edit->textCursor();
		if(originalCursor.position() > 0) {
		    // - place cursor at beginning of document
		    QTextCursor findCursor = QTextCursor(edit->document());
		    findCursor.setPosition(0);
		    edit->setTextCursor(findCursor);

		    // - do a find again
		    // - if find fails this time, set cursor position to original position
		    bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
		    if(!bFound) {
			edit->setTextCursor(originalCursor);
		    } // if we did not find the text anywhere
		} // if cursor was not at top of document
	    } // if not found from cursor
	    // if really not found, show a dialog
	    if(!bFound) {
		//   and produce message dialog
		QMessageBox msgBox;
		msgBox.setText(QString("String '%1' not found in document.").arg(this->lastFindText));
		msgBox.exec();
	    } // show 'not found' dialog
	} // child window active
    } // find dialog clicked 'ok'
}

void CarveWindow::editFindNext() {
    if(!this->activeSVGWindow() || this->lastFindText.length() == 0) { return; }

    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	QPlainTextEdit* edit = childWin->edit();
	bool bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
	if(!bFound) {
	    // if cursor is not at beginning of document:
	    QTextCursor originalCursor = edit->textCursor();
	    if(originalCursor.position() > 0) {
		// - place cursor at beginning of document
		QTextCursor findCursor = QTextCursor(edit->document());
		findCursor.setPosition(0);
		edit->setTextCursor(findCursor);

		// - do a find again
		// - if find fails this time, set cursor position to original position
		bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
		if(!bFound) {
		    edit->setTextCursor(originalCursor);
		} // if we did not find the text anywhere
	    } // if cursor was not at top of document
	} // if not found from cursor
	// if really not found, show a dialog
	if(!bFound) {
	    // and produce message dialog
	    QMessageBox msgBox;
	    msgBox.setText(QString("String '%1' not found in document.").arg(this->lastFindText));
	    msgBox.exec();
	} // show 'not found' dialog
    }
}

void CarveWindow::editFindPrev() {
    if(!this->activeSVGWindow() || this->lastFindText.length() == 0) { return; }

    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	QPlainTextEdit* edit = childWin->edit();
	bool bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
	if(!bFound) {
	    // if cursor is not at end of document:
	    QTextCursor originalCursor = edit->textCursor();
	    if(originalCursor.position() < edit->document()->characterCount()-1) {
		// - place cursor at beginning of document
		QTextCursor findCursor = QTextCursor(edit->document());
		findCursor.setPosition(edit->document()->characterCount()-1);
		edit->setTextCursor(findCursor);

		// - do a find again
		// - if find fails this time, set cursor position to original position
		bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
		if(!bFound) {
		    edit->setTextCursor(originalCursor);
		} // if we did not find the text anywhere
	    } // if cursor was not at top of document
	} // if not found from cursor
	// if really not found, show a dialog
	if(!bFound) {
	    //   and produce message dialog
	    QMessageBox msgBox;
	    msgBox.setText(QString("String '%1' not found in document.").arg(this->lastFindText));
	    msgBox.exec();
	} // show 'not found' dialog
    }
}

void CarveWindow::editReplace() {
    if(!this->activeSVGWindow()) { return; }

    QDialog replaceDlg;
    Ui::FindDialog fui;
    fui.setupUi(&replaceDlg);
    fui.findTextEdit->setText(this->lastFindText);
    fui.replaceTextEdit->setText(this->lastReplaceText);
    fui.findTextEdit->selectAll();
    fui.findTextEdit->setFocus();
    replaceDlg.setWindowTitle("Find & Replace Text");

    if(replaceDlg.exec() == QDialog::Accepted) {
	this->lastFindText = fui.findTextEdit->text();
	this->lastReplaceText = fui.replaceTextEdit->text();
	if(this->lastFindText.length() > 0) {
	    this->ui.actionFindNext->setEnabled(true);
	    this->ui.actionFindPrev->setEnabled(true);
	    this->ui.actionReplaceNext->setEnabled(true);
	}
	else {
	    this->ui.actionFindNext->setEnabled(false);
	    this->ui.actionFindPrev->setEnabled(false);
	    this->ui.actionReplaceNext->setEnabled(false);
	}

	CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
	if(childWin) {
	    QPlainTextEdit* edit = childWin->edit();
	    bool bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
	    if(!bFound) {
		// if cursor is not at beginning of document:
		QTextCursor originalCursor = edit->textCursor();
		if(originalCursor.position() > 0) {
		    // - place cursor at beginning of document
		    QTextCursor findCursor = QTextCursor(edit->document());
		    findCursor.setPosition(0);
		    edit->setTextCursor(findCursor);

		    // - do a find again
		    // - if find fails this time, set cursor position to original position
		    bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
		    if(!bFound) {
			edit->setTextCursor(originalCursor);
		    } // if we did not find the text anywhere
		} // if cursor was not at top of document
	    }
	    // if really not found, show a dialog
	    if(!bFound) {
		//   and produce message dialog
		QMessageBox msgBox;
		msgBox.setText(QString("String '%1' not found in document.").arg(this->lastFindText));
		msgBox.exec();
	    } // show 'not found' dialog
	    else {
		QTextCursor cursor = childWin->edit()->textCursor();
		cursor.insertText(fui.replaceTextEdit->text());
	    }
	}
    }
}

void CarveWindow::editReplaceNext() {
    if(!this->activeSVGWindow() || this->lastFindText.length() == 0) { return; }

    CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(this->activeSVGWindow());
    if(childWin) {
	QPlainTextEdit* edit = childWin->edit();
	bool bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
	if(!bFound) {
	    // if cursor is not at beginning of document:
	    QTextCursor originalCursor = edit->textCursor();
	    if(originalCursor.position() > 0) {
		// - place cursor at beginning of document
		QTextCursor findCursor = QTextCursor(edit->document());
		findCursor.setPosition(0);
		edit->setTextCursor(findCursor);

		// - do a find again
		// - if find fails this time, set cursor position to original position
		bFound = edit->find(this->lastFindText, QTextDocument::FindCaseSensitively);
		if(!bFound) {
		    edit->setTextCursor(originalCursor);
		} // if we did not find the text anywhere
	    } // if cursor was not at top of document
	} // if not found from cursor
	// if really not found, show a dialog
	if(!bFound) {
	    // and produce message dialog
	    QMessageBox msgBox;
	    msgBox.setText(QString("String '%1' not found in document.").arg(this->lastFindText));
	    msgBox.exec();
	} // show 'not found' dialog
	else {
	    QTextCursor cursor = childWin->edit()->textCursor();
	    cursor.insertText(this->lastReplaceText);
	}
    }
}

void CarveWindow::enablePaste() {
    this->ui.actionPaste->setEnabled(true);
}

void CarveWindow::controlPlay() {
    static bool bPreviewWindowShown = false;
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin && childWin->save(lastPath) && !bPreviewWindowShown) {
	bPreviewWindowShown = true;
        // bring up a modal
        cout << childWin->getFilename().toStdString().c_str() << endl;
        CarvePreviewWindow preview(this, childWin->getFilename());
        preview.exec();
	bPreviewWindowShown = false;
    }
}

void CarveWindow::preferences() {
    QString teFont = QString("%1, %2 pt").arg(this->textEditorNewFont.family()).arg(this->textEditorNewFont.pointSize());
    prefUI.textEditorFont->setText(teFont);

    QFont curFont = this->textEditorNewFont;

    // apply preferences
    if(prefDlg->exec() == QDialog::Accepted) {
        // if the selected font is not the same as the current font, then change it
        if(curFont != textEditorNewFont) {
            // iterate over all child windows and change font
            foreach(QMdiSubWindow *window, ui.mdiArea->subWindowList()) {
                CarveSVGWindow* mdiChild = qobject_cast<CarveSVGWindow*>(window->widget());
                mdiChild->setFont(this->textEditorNewFont);
            }

        }
    }
    else { // else, reset the new font to the current font
        textEditorNewFont = curFont;
    }
}

void CarveWindow::chooseNewEditorFont() {
    QFontDialog fdlg(this->textEditorNewFont, prefDlg);
    fdlg.setOption(QFontDialog::DontUseNativeDialog, true);
    if(fdlg.exec() == QDialog::Accepted) {
        this->textEditorNewFont = fdlg.selectedFont();
        QString teFont = QString("%1, %2 pt").arg(this->textEditorNewFont.family()).arg(this->textEditorNewFont.pointSize());
        prefUI.textEditorFont->setText(teFont);
    }
}

void CarveWindow::helpAbout() {
    QDialog helpDlg;
    Ui::HelpDialog ui;

    ui.setupUi(&helpDlg);
    QString verstr = QString("Version %1 (%2), Build %3").arg(SW_VERSION).arg(SW_CODENAME).arg(SW_BUILD);
    ui.labelVersion->setText(verstr);

    helpDlg.exec();
}

void CarveWindow::updateWindowMenu(QMdiSubWindow* window) {
    static CarveSVGWindow* prevWindow = 0;
    if(window) {
        this->updateDocumentMenus();
        CarveSVGWindow* childWin = qobject_cast<CarveSVGWindow*>(window->widget());
        if(childWin && childWin != prevWindow) {
            refreshXMLStatus(false);

            // change the DOM Browser tree's model to the new child window's
	    domTree->setWindow(childWin);
	    // expand all nodes
            domTree->expandAll();
            // set column 0 width the first time
            if(domBrowserColumn0Width != -1) {
                domTree->setColumnWidth(0, domBrowserColumn0Width);
                domTree->setColumnWidth(1, domTree->width()-domTree->columnWidth(0));
            }

	    this->selectNode(NULL);

	    // change mode switching button
	    if(childWin->mode() == Code) {
		ui.actionSwitchMode->setToolTip("Click to switch to Design mode");
		ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/design.png"));
	    }
	    else {
		ui.actionSwitchMode->setToolTip("Click to switch to Code Editing mode");
		ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/code.png"));
	    }

	    prevWindow = childWin;
        }
    }
    else {
        this->timerDocModified->stop();
        labelXML->setText(szXMLDisabled);
        labelXML->setToolTip("Indicates whether the SVG document is valid XML.");
    }
}

void CarveWindow::updateDocumentMenus() {
    // if no child windows, then disable Close and Close All
    int numChildren = ui.mdiArea->subWindowList().size();
    if(numChildren > 0) {
        ui.actionFileSave->setDisabled(false);
        ui.actionFileSaveAs->setDisabled(false);
	ui.actionExportAsPNG->setDisabled(false);
	ui.menuExport->setDisabled(false);
        ui.actionFileClose->setDisabled(false);
        ui.actionFileCloseAll->setDisabled(false);
        ui.menuActiveWindow->menuAction()->setVisible(true);

        CarveSVGWindow* activeWin = this->activeSVGWindow();

        // display edit, control menus
        ui.menuEdit->menuAction()->setVisible(true);
        ui.menuControl->menuAction()->setVisible(true);
	ui.actionSwitchMode->setVisible(true);
	ui.actionPlay->setVisible(true);

        // change the currently active document
        ui.menuActiveWindow->setTitle(activeWin->windowTitle());

        // update the window submenu for the list of documents
        ui.menuActiveWindow->clear();

        foreach(QMdiSubWindow *window, ui.mdiArea->subWindowList()) {
            CarveSVGWindow* mdiChild = qobject_cast<CarveSVGWindow*>(window->widget());
            QAction* theAction = NULL;
            if(!mdiChild->isUntitled()) {
                theAction = ui.menuActiveWindow->addAction(QFileInfo(mdiChild->getFilename()).fileName());
            }
            else {
                theAction = ui.menuActiveWindow->addAction(mdiChild->windowTitle().left(mdiChild->windowTitle().size()-4));
            }
            theAction->setCheckable(true);
//            theAction->setData(mdiChild);
            if(mdiChild == this->activeSVGWindow()) {
                theAction->setChecked(true);
            }
        }
	ui.menuMode->menuAction()->setVisible(true);
	ui.actionCode->setChecked(activeWin->mode() == Code);
	ui.actionDesign->setChecked(activeWin->mode() == Design);
    }
    else {
        ui.actionFileSave->setDisabled(true);
        ui.actionFileSaveAs->setDisabled(true);
	ui.actionExportAsPNG->setDisabled(true);
	ui.menuExport->setDisabled(true);
        ui.actionFileClose->setDisabled(true);
        ui.actionFileCloseAll->setDisabled(true);
        ui.menuActiveWindow->menuAction()->setVisible(false);
        ui.menuEdit->menuAction()->setVisible(false);
        ui.menuControl->menuAction()->setVisible(false);
	ui.actionSwitchMode->setVisible(false);
	ui.actionPlay->setVisible(false);
	ui.menuMode->menuAction()->setVisible(false);
    }
}

void CarveWindow::activeDocumentWasModified() {
    this->timerDocModified->start(REFRESH_XML_TIMER);
}

void CarveWindow::refreshXMLStatus(bool bRebuild) {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin) {
	bool bValid = !childWin->model()->domDocument()->documentElement().isNull();
	if(bRebuild) {
	    this->propPane->setNode(NULL);

            // calling isValidXML() rebuilds everything
            bValid = childWin->isValidXML();
            // expand all nodes
            domTree->expandAll();
        }
        if(bValid) {
            labelXML->setText( szXMLValid );
            labelXML->setToolTip(tr("The SVG document is valid XML."));
        }
        else {
            labelXML->setText( szXMLInvalid );
            labelXML->setToolTip(tr("The SVG document is invalid XML!"));
        }
        this->timerDocModified->stop();
    }
}

void CarveWindow::domBrowserDocked(Qt::DockWidgetArea area) {
    bDOMBrowserDockedLeft = (area == Qt::LeftDockWidgetArea);
}

void CarveWindow::updateRecentFileActions() {
    QSettings settings("codedread", "Carve");
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for(int i = 0; i < numRecentFiles; ++i) {
	QString text = tr("%1").arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for(int j = numRecentFiles; j < MaxRecentFiles; ++j) {
        recentFileActs[j]->setVisible(false);
    }

    ui.menuRecentFiles->menuAction()->setVisible(numRecentFiles > 0);
}

void CarveWindow::setCurrentFile(const QString &fileName)
{
    QSettings settings("codedread", "Carve");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while(files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);

    updateRecentFileActions();
}

void CarveWindow::domBrowserHeaderChanged(int logicalIndex, int, int newSize) {
    if(logicalIndex == 0) {
	this->domBrowserColumn0Width = newSize;
    }
}

void CarveWindow::selectNode(CarveSVGNode* node) {
    emit nodeSelected(node);
}

void CarveWindow::deleteNode(CarveSVGNode* node) {
    CarveSVGWindow* window = this->activeSVGWindow();
    if(!window) {
	cout << "Error!  Deleting a node without an active window" << endl;
    }

    // clear the Properties pane
    this->propPane->setNode(NULL);

    QDomElement nodeToDelete = node->domElem();
    QDomElement rootNode = window->model()->domDocument()->documentElement();
    // TODO: ensure document is marked as modified
    if(nodeToDelete == rootNode) {
	// remove entire document (but make it undo-able)
	QTextCursor cursor(window->edit()->document());
	cursor.beginEditBlock();
	cursor.select(QTextCursor::Document);
	cursor.removeSelectedText();
	cursor.endEditBlock();

	// clear the DOM Browser
	window->model()->domDocument()->clear();
    }
    else {
	// remove the DomElement
	nodeToDelete.parentNode().removeChild(nodeToDelete);

	// serialize the DOM and set the document's text (but make it undo-able)
	QTextCursor cursor(window->edit()->document());
	cursor.beginEditBlock();
	cursor.select(QTextCursor::Document);
	cursor.removeSelectedText();
	// insert the reserialized DOM
	// TODO: do a diff between the previous text contents and the reserialized DOM
	// and then only change this text (this should reduce memory considerably)
	// TODO: make this default indentation a setting
	cursor.insertText(window->model()->domDocument()->toString(2));
	cursor.endEditBlock();
    }

    this->updateDocImmediately();

    // not sure this is needed
    emit nodeDeleted(node);
}

// TODO: Make a common function that sets the mode, does the menu item checking, changes the icon, etc

void CarveWindow::switchMode() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin) {
	SVGWindowMode newMode = (childWin->mode() == Code ? Design : Code);
	ui.actionCode->setChecked(newMode == Code);
	ui.actionDesign->setChecked(newMode == Design);
	if(newMode == Code) {
	    ui.actionSwitchMode->setToolTip("Click to switch to Design mode");
	    ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/design.png"));
	}
	else {
	    ui.actionSwitchMode->setToolTip("Click to switch to Code Editing mode");
	    ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/code.png"));
	}
	childWin->switchMode(newMode);
    }
}

void CarveWindow::modeCodeClicked() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin && childWin->mode() == Design) {
	childWin->switchMode(Code);
    }
    ui.actionCode->setChecked(true);
    ui.actionDesign->setChecked(false);
    ui.actionSwitchMode->setToolTip("Click to switch to Design mode");
    ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/design.png"));
}

void CarveWindow::modeDesignClicked() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin && childWin->mode() == Code) {
	childWin->switchMode(Design);
    }
    ui.actionCode->setChecked(false);
    ui.actionDesign->setChecked(true);
    ui.actionSwitchMode->setToolTip("Click to switch to Code editing mode");
    ui.actionSwitchMode->setIcon(QIcon(":/toolbars/images/code.png"));
}

void CarveWindow::updateDocImmediately() {
    this->timerDocModified->start(0);
}

void CarveWindow::fileExportPNG() {
    CarveSVGWindow* childWin = this->activeSVGWindow();
    if(childWin && childWin->save(lastPath)) {
	// create QGraphicsView with the SVG loaded in
	QGraphicsView view;
	view.setScene(new QGraphicsScene(&view));
	QGraphicsSvgItem* svgItem = new QGraphicsSvgItem(childWin->getFilename());
	svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
	svgItem->setCacheMode(QGraphicsItem::NoCache);
	svgItem->setZValue(0);
	view.scene()->addItem(svgItem);

	// create a QImage with the scene size and format QImage::Format_ARGB32
	QImage img((int)view.sceneRect().width(), (int)view.sceneRect().height(), QImage::Format_ARGB32);
	img.fill(0);
	QPainter painter(&img);

	// paint the scene to the image
	view.scene()->render(&painter);

	// ask the user for a filename (with PNG extension)
	QFileInfo finfo(childWin->getFilename());
	QString path(lastPath); path.append("/").append(finfo.baseName()).append(".png");
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export As PNG"), path, "PNG Files (*.png)");
	if (fileName.isEmpty()) {
	    return;
	}

	// finally save the PNG
	cout << "path = " << path.toStdString().c_str() << endl;
	cout << "filename = " << fileName.toStdString().c_str() << endl;
	img.save(fileName);
    }
}
