#pragma once
#include <QtCharts/QChart>
#include <QtCharts/QChartGlobal>

#include <vector>

QT_CHARTS_USE_NAMESPACE

namespace ChartCore
{

class GraphicsTextItem;

class ChartTracker;


class Chart : public QChart
{
	Q_OBJECT
public:
	Chart(QChart * parent = nullptr);
	~Chart();

	bool addSeries(QAbstractSeries *);
	void addTracker(const ChartTracker *);
	void changeTrackerValue(ChartTracker *, QVariant value);

	void drawGrid();

	void enabledHorizontalLabel(bool enabled = true);

	QAbstractAxis * getHorizontal() const;
	
	QDateTime getMaxDateHorizontal() const;
	QDateTime getMinDateHorizontal() const;

	double getOffsetTop() const;
	double getOffsetRight() const;


	void scrollTo(const QVariant & value, Qt::Orientation orientation = Qt::Horizontal);

	void setHorizontal(QAbstractAxis *);
	void setHorizontalType(QAbstractAxis::AxisType type);
	void setInfo(const QString &infoText);
	void setOffsetRight(qreal);
	
	void setOffsetTop(qreal);
	void setVerticalType(QAbstractAxis::AxisType);
	void setVerticalTick(uint);
	void setVerticalTickValue(std::vector<double> &);

public slots:
	void setRangeHorizontal(const QVariant &max, const QVariant &min);
	void setRangeVertical(const QVariant &max, const QVariant &min);

private:
	QAbstractAxis * horizontal = nullptr;
	QAbstractAxis * vertical   = nullptr;

	ChartCore::GraphicsTextItem *info = nullptr;

	uint maxHorizontalTickCount = 10;
	uint maxVerticalTickCount   = 5;

	std::vector<double> ticks;

	std::vector<QGraphicsLineItem *> gridItem;
	std::vector<QGraphicsSimpleTextItem *> gridLabels;

	bool horizontalLabelEnabled = false;
	bool changeOffset = false;

	QString dateFormat = "MM/dd HH:mm";

private slots:
	void plotChanges(const QRectF &);

};



}