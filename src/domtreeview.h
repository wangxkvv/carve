#ifndef DOMTREEVIEW_H
#define DOMTREEVIEW_H

#include <QTreeView>
class QContextMenuEvent;
class QMenu;
class CarveWindow;
class CarveSVGWindow;
class CarveSVGNode;

class DomTreeView : public QTreeView
{
    Q_OBJECT

public:
    DomTreeView(CarveWindow* window, QSize hint);
    QSize sizeHint() const { return hint_; }
    void setWindow(CarveSVGWindow* w);
protected:
    void contextMenuEvent(QContextMenuEvent* e);
private:
    QSize hint_;
    QMenu* menuContext;
    QAction* actionDeleteNode;
    CarveWindow* mainwindow;
    CarveSVGWindow* window;

    bool findIndex(CarveSVGNode* node, QModelIndex& index);

public slots:
    void nodeClicked(const QModelIndex& index);
    void nodeSelected(CarveSVGNode* node);
};

#endif // DOMTREEVIEW_H
