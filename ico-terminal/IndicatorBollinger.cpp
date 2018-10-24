#include "Calculation.h"
#include "IndicatorBollinger.h"
#include "TradePrice.h"

#include <QtCharts\QLineSeries>

namespace Indicators
{


IndicatorBollinger::IndicatorBollinger(AbstractIndicator * parent) : AbstractIndicator(parent), topSeries(new QLineSeries), buttomSeries(new QLineSeries)
{
	seriesList.push_back(topSeries);
	seriesList.push_back(buttomSeries);
	type = AbstractIndicator::Bollinger;
	alias = "Bollinger bands";
}

IndicatorBollinger::~IndicatorBollinger()
{
	topSeries->deleteLater();
	buttomSeries->deleteLater();
}

QAbstractSeries * IndicatorBollinger::getTopLine() const
{
	return topSeries;
}

QAbstractSeries * IndicatorBollinger::getBottomLine() const
{
	return buttomSeries;
}

void IndicatorBollinger::setPeriod(uint p)
{
	period = p;
}

uint IndicatorBollinger::getPeriod() const
{
	return period;
}

void IndicatorBollinger::setMultiplier(uint v)
{
	multiplier = v;
}

uint IndicatorBollinger::getMultiplier() const
{
	return multiplier;
}

QColor IndicatorBollinger::getColor() const
{
	return topSeries->color();
}

int IndicatorBollinger::getSize() const
{
	return topSeries->pen().width();;
}

void IndicatorBollinger::process(const std::vector<Entity::TradePrice> &tradeList)
{
	clear();
	for (const auto &trade : tradeList) append(trade);
}

void IndicatorBollinger::append(const Entity::TradePrice &trade)
{
	temporary.push_back(trade);

	if (temporary.size() > period)
	{
		const auto middle = Calculation::Sma(temporary.crbegin(), temporary.crend(), period);
		const auto date   = trade.getDate() * 1000;
		const auto sigma  = Calculation::StandardDeviation(temporary.crbegin(), temporary.crend(), middle, period);

		topSeries->append(QPointF(date,middle + multiplier * sigma));
		buttomSeries->append(QPointF(date,middle - multiplier * sigma));

		if (temporary.size() > period + 1)
		{
			temporary.pop_front();
		}
	}
}

void IndicatorBollinger::removeFirst()
{
	if (!topSeries || !topSeries->points().size()) return;

	topSeries->removePoints(0,1);
	buttomSeries->removePoints(0, 1);
	if (temporary.size() < period)
	{
		temporary.pop_front();
	}
}

void IndicatorBollinger::removeLast()
{
	const auto size = topSeries->points().size();
	if (!topSeries || !buttomSeries || !size) return;

	topSeries->removePoints(size - 1, 1);
	buttomSeries->removePoints(size - 1, 1);

	temporary.pop_back();
}

void IndicatorBollinger::clear()
{
	temporary.clear();
	topSeries->clear();
	buttomSeries->clear();
}

void IndicatorBollinger::changePeriod(uint p)
{
	setPeriod(p);
	emit requestData();
}

void IndicatorBollinger::changeMultiplier(uint v)
{
	setMultiplier(v);
	emit requestData();
}

void IndicatorBollinger::changeColor(const QColor &color)
{
	topSeries->setColor(color);
	buttomSeries->setColor(color);
}

void IndicatorBollinger::changeSize(int widthPen)
{
	auto seriesPen = topSeries->pen();
	seriesPen.setWidth(widthPen);

	topSeries->setPen(seriesPen);
	buttomSeries->setPen(seriesPen);
}


}