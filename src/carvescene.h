#ifndef CARVESCENE_H
#define CARVESCENE_H

#include <QGraphicsScene>

class CarveWindow;
class CarveSVGNode;

class CarveScene : public QGraphicsScene
{
    Q_OBJECT

public:
    CarveScene(CarveWindow* window);

    CarveWindow* mainwindow() { return mainwindow_; }

private:
    CarveWindow* mainwindow_;

private slots:
    // when user clicks to select in Design mode
    void selectNode();

    // when user has selected a node elsewhere
    void nodeSelected(CarveSVGNode* node);
    void nodeDeleted(CarveSVGNode* node);
};

#endif // CARVESCENE_H
