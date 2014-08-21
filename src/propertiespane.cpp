#include "propertiespane.h"
#include "carvesvgnode.h"
#include "domhelper.h"

#include <QFrame>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QDomText>

#include <iostream>
using std::cout;
using std::endl;

QHash<int, QStringList> PropertiesPane::properties;

PropertiesPane::PropertiesPane(const QSize& size) :
	QScrollArea()
{
    // these are the default properties shown in the properties panel
    if(this->properties.isEmpty()) {
	properties[svgSvg]
		<< "width"
		<< "height"
		<< "viewBox"
		<< "version"
		<< "baseProfile"
		<< "id"
		<< "style"
		;
        properties[svgG]
                << "id"
                << "transform"
                << "fill"
                << "fill-opacity"
                << "stroke"
                << "stroke-opacity"
                << "font-size"
		<< "style"
		;
	properties[svgRect]
		<< "id"
		<< "x"
		<< "y"
		<< "width"
		<< "height"
		<< "rx"
		<< "ry"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgCircle]
		<< "id"
		<< "cx"
		<< "cy"
		<< "r"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgEllipse]
		<< "id"
		<< "cx"
		<< "cy"
		<< "rx"
		<< "ry"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgLine]
		<< "id"
		<< "x1"
		<< "y1"
		<< "x2"
		<< "y2"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgPolyline]
		<< "id"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgPolygon]
		<< "id"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgPath]
		<< "id"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgLinearGradient]
		<< "id"
		<< "x1"
		<< "y1"
		<< "x2"
		<< "y2"
		<< "gradientUnits"
//		<< "xlink:href"
		;
	properties[svgRadialGradient]
		<< "id"
		<< "cx"
		<< "cy"
		<< "r"
//		<< "fx"
//		<< "fy"
		<< "gradientUnits"
//		<< "xlink:href"
		;
	properties[svgStop]
		<< "offset"
		<< "stop-color"
		<< "stop-opacity"
		<< "style"
		;
	properties[svgText]
		<< "id"
		<< "#text"
		<< "x"
		<< "y"
		<< "font-size"
		<< "font-weight"
		<< "font-family"
		<< "fill"
		<< "fill-opacity"
		<< "stroke"
		<< "stroke-opacity"
		<< "stroke-width"
		<< "style"
		;
	properties[svgA]
		<< "xlink:href"
		<< "xlink:title"
		<< "style"
		;
	properties[svgImage]
		<< "id"
		<< "xlink:href"
		<< "x"
		<< "y"
		<< "width"
		<< "height"
		<< "preserveAspectRatio"
		<< "style"
		;
    }

    this->resize(size);
    propPane = new QFrame();
    propPane->resize(size);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setWidget(propPane);
}

void PropertiesPane::setNode(CarveSVGNode* node) {
    if(this->node_ == node) { return; }

    // remove all UI elements
    QFormLayout* layout = dynamic_cast<QFormLayout*>(propPane->layout());
    int i = this->labels.length() - 1;
    while(i >= 0) {
	layout->removeWidget(this->labels.at(i));
	layout->removeWidget(this->edits.at(i));
	delete this->labels.at(i);
	delete this->edits.at(i);
	i--;
    }
    this->labels.clear();
    this->edits.clear();

    this->node_ = node;

    if(node == NULL) { return; }

    if(node) {
	createFields((int)node_->type());
    }
}

void PropertiesPane::createFields(int type) {
    this->setWidget(NULL);
    if(propPane->layout()) delete propPane->layout();
    delete propPane;

    propPane = new QFrame();
    if(this->properties.contains(type)) {
	QFormLayout* newLayout = new QFormLayout;
	QStringList& propList = properties[(int)type];
	for(int i = 0; i < propList.size(); ++i) {
	    QLabel* theLabel = new QLabel(propList[i]);
	    QLineEdit* theEdit = NULL;
	    if(propList[i] == "#text") {
		theEdit = new QLineEdit(node_->domElem().text());
	    }
	    else {
		theEdit = new QLineEdit(node_->domElem().attributes().namedItem(propList[i]).nodeValue());
	    }
	    this->labels.append(theLabel);
	    this->edits.append(theEdit);
	    connect(theEdit, SIGNAL(editingFinished()), this, SLOT(fieldChanged()));
	    newLayout->addRow(theLabel, theEdit);
	}
	propPane->setLayout(newLayout);
    }

    this->setWidget(propPane);
}

void PropertiesPane::fieldChanged() {
    cout << "field changed" << endl;
    if(!node_) { return; }

    QFormLayout* layout = qobject_cast<QFormLayout*>(propPane->layout());
    QLineEdit* theEdit = qobject_cast<QLineEdit*>(sender());
    if(layout && theEdit) {
	QLabel* theLabel = qobject_cast<QLabel*>(layout->labelForField(theEdit));
	if(theLabel) {
	    if(theLabel->text() == "#text") {
		// clear out children
		QDomElement elem = node_->domElem();
		while(elem.hasChildNodes()) {
		    elem.removeChild(node_->domElem().lastChild());
		}
		elem.appendChild(elem.ownerDocument().createTextNode(theEdit->text()));
	    }
	    else {
		cout << "Set trait" << endl;
		node_->setTrait(theLabel->text(), theEdit->text());
	    }
	}
    }
}
