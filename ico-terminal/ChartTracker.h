#pragma once

#include <Qt>
#include <QtCharts/QChartGlobal>

#include <QColor>
#include <QLineF>

QT_BEGIN_NAMESPACE
class QGraphicsLineItem;
class QGraphicsScene;
class QPen;
QT_END_NAMESPACE


namespace ChartCore
{


class PriceGraphicsTextItem;
using LabelGrid = PriceGraphicsTextItem; //TODO crutch


class ChartTracker
{
public:
	ChartTracker();
	~ChartTracker();

	void addToScene(QGraphicsScene *) const;

	Qt::Orientation getOrientation() const;

	void hide();

	void setBackgroundColor(const QColor &);
	void setFontColor(const QColor &);
	void setLabelSize(qreal);
	void setLine(const QLineF &);
	void setOrientation(Qt::Orientation);
	void setPen(const QPen &);
	void setText(const QString &);
	
	void show();

private:	
	LabelGrid * label = nullptr;
	QGraphicsLineItem * line = nullptr;

	Qt::Orientation orientation = Qt::Horizontal;

	bool hideLabel = false;
};


}



