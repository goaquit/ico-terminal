#pragma once
#include "AbstractIndicator.h"
#include "TradePrice.h"
#include <QtCharts\QChartGlobal>

#include <deque>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Indicators
{


class IndicatorSma : public AbstractIndicator
{
public:
	IndicatorSma(AbstractIndicator *parent = nullptr);
	~IndicatorSma();

	void setPeriod(uint);

	QAbstractSeries * getSeriesSma() const;

	// Inherited via AbstractIndicator
	virtual void process(const std::vector<Entity::TradePrice>&) override;
	virtual void append(const Entity::TradePrice &) override;
	virtual void removeFirst() override;
	virtual void removeLast() override;
	virtual void clear() override;

	// getting parameters
	uint getPeriod() const;
	QColor getColor() const;
	int getSize() const;

	// changes parameters
	void changePeriod(uint);
	void changeColor(const QColor &);
	void changeSize(int);

private:
	QLineSeries * series = nullptr;
	uint period = 20;

	std::deque<Entity::TradePrice> temporary;
};


}


