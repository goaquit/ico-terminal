#pragma once
#include "AbstractIndicator.h"
#include <QtCharts\QChartGlobal>

#include <deque>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Indicators
{


class IndicatorBollinger : public AbstractIndicator
{
public:
	IndicatorBollinger(AbstractIndicator *parent = nullptr);
	~IndicatorBollinger();


	QAbstractSeries *getTopLine() const;
	QAbstractSeries *getBottomLine() const;

	void setPeriod(uint);
	uint getPeriod() const;

	void setMultiplier(uint);
	uint getMultiplier() const;

	QColor getColor() const;
	int getSize() const;

	virtual void process(const std::vector<Entity::TradePrice>&) override;
	virtual void append(const Entity::TradePrice &) override;
	virtual void removeFirst() override;
	virtual void removeLast() override;
	virtual void clear() override;

	// changes parameters
	void changePeriod(uint);
	void changeMultiplier(uint);
	void changeColor(const QColor &);
	void changeSize(int);

private:
	QLineSeries *topSeries;
	QLineSeries *buttomSeries;

	uint period = 20;
	uint multiplier = 2;

	std::deque<Entity::TradePrice> temporary;
};


}