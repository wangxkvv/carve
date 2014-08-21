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
#ifndef CARVEWINDOW_H
#define CARVEWINDOW_H

#include "carve.h"
#include <QtGui/QMainWindow>
#include <QSettings>
#include <QVector>
#include <QMdiArea>
#include <QFont>
#include "../ui_carvewindow.h"

class CarveSVGDocument;
class CarveSVGNode;
class CarveSVGWindow;
class QCloseEvent;
class QLabel;
class QTimer;
class QDialog;
class QTreeView;
class DomTreeView;
class PropertiesPane;

namespace Ui {
    class PreferencesDialog;
};

class CarveWindow : public QMainWindow
{
    Q_OBJECT

public:
    CarveWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

    // global actions that affect a lot of things
    void selectNode(CarveSVGNode* node);
    void deleteNode(CarveSVGNode* node);

    void updateDocImmediately();
    CarveSVGWindow* activeSVGWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::CarveWindowClass ui;

    // persistent settings
    QPoint windowPos;
    QSize windowSize;
    QString lastPath;
    bool bDOMBrowserDockedLeft;
    enum { MaxRecentFiles = 8 };
    QAction *recentFileActs[MaxRecentFiles];
    int domBrowserColumn0Width;

    QLabel* labelXML;
    QTimer* timerDocModified;
    QFont textEditorNewFont;
    QDialog* prefDlg;
    DomTreeView* domTree;
    QString lastFindText;
    QString lastReplaceText;
    PropertiesPane* propPane;

    void loadSettings();
    void saveSettings();

    QMdiSubWindow* findChildWindow(const QString& filename);
    CarveSVGWindow* createMDIChild();
    void prepareNewChildWindow(CarveSVGWindow* childWin);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();

signals:
    void nodeSelected(CarveSVGNode* node);
    void nodeDeleted(CarveSVGNode* node);

private slots:
    void fileNew();
    void fileOpen();
    void fileOpenRecentFile();
    void fileSave();
    void fileSaveAs();
    void fileExportPNG();
    void fileQuit();
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editFind();
    void editFindNext();
    void editFindPrev();
    void editReplace();
    void editReplaceNext();
    void enablePaste();
    void controlPlay();
    void preferences();
    void helpAbout();

    void updateWindowMenu(QMdiSubWindow* window);
    void updateDocumentMenus();
    void activeDocumentWasModified();
    void refreshXMLStatus(bool bRebuild = true);
    void chooseNewEditorFont();
    void domBrowserDocked(Qt::DockWidgetArea area);
    void domBrowserHeaderChanged(int logicalIndex, int oldSize, int newSize);
//    void propPaneDocked(Qt::DockWidgetArea area);

    void switchMode();
    void modeCodeClicked();
    void modeDesignClicked();
};

#endif // CARVEWINDOW_H
