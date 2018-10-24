#pragma once
#include "AbstractIndicator.h"

#include <QtCharts\QChartGlobal>

#include <deque>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
QT_CHARTS_END_NAMESPACE

using namespace Entity;

namespace Indicators
{


class IndicatorStochastic : public AbstractIndicator
{
public:
	IndicatorStochastic(AbstractIndicator *parent = nullptr);
	~IndicatorStochastic();

	// Inherited via AbstractIndicator
	virtual void process(const std::vector<Entity::TradePrice>&) override;
	virtual void append(const Entity::TradePrice &) override;
	virtual void removeFirst() override;
	virtual void removeLast() override;
	virtual void clear() override;

	void setFastPeriod(uint);
	void setSlowPeriod(uint);
	void setSmooth(uint);

	// getting parameter
	uint getFastPeriod() const;
	uint getSlowPeriod() const;
	uint getSmooth() const;

	QColor getFastColor() const;
	QColor getSlowColor() const;

	int getFastSize() const;
	int getSlowSize() const;

	// changes parameter
	void changeFast(uint);
	void changeSlow(uint);
	void changeSmooth(uint);

	void changeFastColor(const QColor &);
	void changeSlowColor(const QColor &);

	void changeFastSize(int);
	void changeSlowSize(int);

private:
	QLineSeries *kLine = nullptr; // main line
	QLineSeries *dLine = nullptr; // second MA line

	uint fastPeriod = 0;
	uint slowPeriod = 0;
	uint smooth     = 3;

	// temporary data
	std::deque<Entity::TradePrice> tradeInPeriod;
	std::deque<double> kValue;
	std::deque<double> kSmoothValues;
};



}