#include "Calculation.h"
#include "IndicatorStochastic.h"
#include <QtCharts\QLineSeries>

namespace Indicators
{


IndicatorStochastic::IndicatorStochastic(AbstractIndicator *parent) 
	: AbstractIndicator(parent),
	kLine(new QLineSeries(this)),dLine(new QLineSeries(this))
{
	seriesList.push_back(kLine);
	seriesList.push_back(dLine);

	kLine->setPen(QPen(Qt::green, 2));
	dLine->setPen(QPen(Qt::red, 2, Qt::DashLine));

	type = AbstractIndicator::Stochastic;
	alias = "Stochastic";
}


IndicatorStochastic::~IndicatorStochastic()
{
	kLine->deleteLater();
	dLine->deleteLater();
}

void IndicatorStochastic::process(const std::vector<Entity::TradePrice>&)
{
	//nothing
}

void IndicatorStochastic::append(const Entity::TradePrice &trade)
{
	if (!fastPeriod) return;

	tradeInPeriod.push_back(trade);
	if (tradeInPeriod.size() < fastPeriod) return;
	
	kValue.push_back(Calculation::FastStochastic(tradeInPeriod, tradeInPeriod.crbegin(), fastPeriod));

	// smooth stochastic
	const auto kSmooth = Calculation::Sma(kValue, smooth);
	kSmoothValues.push_back(kSmooth);

	// slow stochastic
	const auto d = Calculation::Sma(kSmoothValues, slowPeriod);

	const auto time = trade.getDate() * 1000;

	kLine->append(time, kSmooth);
	dLine->append(time, d);

	if (kValue.size() > slowPeriod)
	{
		kValue.pop_front();
		kSmoothValues.pop_front();
	}

	if (tradeInPeriod.size() > fastPeriod)
	{
		tradeInPeriod.pop_front();
	}
}

void IndicatorStochastic::removeFirst()
{
	const auto sizeK = kLine->points().size();
	const auto sizeD = dLine->points().size();

	if (sizeK <= 1 || sizeD <= 1)
	{
		clear();

		return;
	}

	tradeInPeriod.pop_front();
	kValue.pop_front();
	kSmoothValues.pop_front();

	kLine->removePoints(0, 1);
	dLine->removePoints(0, 1);
}

void IndicatorStochastic::removeLast()
{
	const auto sizeK = kLine->points().size();
	const auto sizeD = dLine->points().size();

	if (sizeK <= 1 || sizeD <= 1)
	{
		clear();

		return;
	}

	tradeInPeriod.pop_back();
	kValue.pop_back();
	kSmoothValues.pop_back();

	kLine->removePoints(sizeK - 1, 1);
	dLine->removePoints(sizeD - 1, 1);
}

void IndicatorStochastic::clear()
{
	tradeInPeriod.clear();
	kValue.clear();
	kSmoothValues.clear();

	kLine->clear();
	dLine->clear();
}

void IndicatorStochastic::setFastPeriod(uint p)
{
	fastPeriod = p;
}

void IndicatorStochastic::setSlowPeriod(uint p)
{
	slowPeriod = p;
}

void IndicatorStochastic::setSmooth(uint v)
{
	smooth = v;
}

uint IndicatorStochastic::getFastPeriod() const
{
	return fastPeriod;
}

uint IndicatorStochastic::getSlowPeriod() const
{
	return slowPeriod;
}

uint IndicatorStochastic::getSmooth() const
{
	return smooth;
}

QColor IndicatorStochastic::getFastColor() const
{
	return kLine->color();
}

QColor IndicatorStochastic::getSlowColor() const
{
	return dLine->color();
}

int IndicatorStochastic::getFastSize() const
{
	return kLine->pen().width();
}

int IndicatorStochastic::getSlowSize() const
{
	return dLine->pen().width();
}

void IndicatorStochastic::changeFast(uint v)
{
	setFastPeriod(v);
	emit requestData();
}

void IndicatorStochastic::changeSlow(uint v)
{
	setSlowPeriod(v);
	emit requestData();
}

void IndicatorStochastic::changeSmooth(uint v)
{
	setSmooth(v);

	emit requestData();
}

void IndicatorStochastic::changeFastColor(const QColor &v)
{
	kLine->setColor(v);
}

void IndicatorStochastic::changeSlowColor(const QColor &v)
{
	dLine->setColor(v);
}

void IndicatorStochastic::changeFastSize(int v)
{
	auto p = kLine->pen();
	p.setWidth(v);
	kLine->setPen(p);
}

void IndicatorStochastic::changeSlowSize(int v)
{
	auto p = dLine->pen();
	p.setWidth(v);
	dLine->setPen(p);
}


}