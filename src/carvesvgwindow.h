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
#ifndef CARVESVGWINDOW_H
#define CARVESVGWINDOW_H

#include <QPlainTextEdit>
#include <QDir>
#include <QStackedWidget>

class CarveSVGDocument;
class CarveDesignView;
class CarveScene;
class CarveWindow;

enum SVGWindowMode { Code, Design };

// This is the individual SVG document window set into the MDI area
class CarveSVGWindow : public QStackedWidget
{
    Q_OBJECT

public:
    CarveSVGWindow(CarveWindow* window);

    bool isUntitled() const { return untitled_; }
    const QString& getFilename() const { return filename_; }
    CarveSVGDocument* model();
    const QDir& getAbsoluteDir() const { return dir_; }
    bool isValidXML(); // WARNING: this actually causes a re-build of the scene and model!
    QPlainTextEdit* edit() { return edit_; }
    SVGWindowMode mode() const { return mode_; }
    void switchMode(SVGWindowMode newMode);
    CarveScene* scene() { return scene_; }
    CarveDesignView* view() { return view_; }
    CarveWindow* mainwindow() { return mainwindow_; }

    bool load(const QString& filename);
    bool save(const QString& lastPath);
    bool saveAs(const QString& lastPath);

    void updateDocImmediately();
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void documentWasModified();

private:
    bool untitled_;
    QString filename_;
    CarveSVGDocument* model_;
    QDir dir_;
    QPlainTextEdit* edit_;
    SVGWindowMode mode_;
    CarveDesignView* view_;
    CarveScene* scene_;
    CarveWindow* mainwindow_;

    void init();
    bool saveFile();
};

#endif // CARVESVGWINDOW_H
