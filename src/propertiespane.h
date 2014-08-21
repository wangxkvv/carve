#ifndef PROPERTIESPANE_H
#define PROPERTIESPANE_H

#include <QScrollArea>
#include <QList>
#include <QHash>
#include <QStringList>

class QFrame;
class CarveSVGNode;
class QLabel;
class QLineEdit;

class PropertiesPane : public QScrollArea
{
    Q_OBJECT

public:
    PropertiesPane(const QSize& size);

    CarveSVGNode* selectedNode() const { return node_; }

public slots:
    void setNode(CarveSVGNode* node);

private:
    QFrame* propPane;
    CarveSVGNode* node_;
    QList<QLabel*> labels;
    QList<QLineEdit*> edits;
    static QHash<int, QStringList> properties;

    void createFields(int type);

private slots:
    void fieldChanged();
};

#endif // PROPERTIESPANE_H
