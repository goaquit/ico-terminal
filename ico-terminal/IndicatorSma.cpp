#include "Calculation.h"
#include "Chart.h"
#include "IndicatorSma.h"
#include "TradePrice.h"

#include <QtCharts\QLineSeries>

#include <vector>

using namespace ChartCore;
using namespace Entity;

namespace Indicators
{

IndicatorSma::IndicatorSma(AbstractIndicator * parent) : AbstractIndicator(parent), series(new QLineSeries)
{
	seriesList.push_back(series);
	type = AbstractIndicator::Sma;
	alias = "Sma";
}

IndicatorSma::~IndicatorSma()
{
	series->deleteLater();
}

void IndicatorSma::setPeriod(uint p)
{
	period = p;
}

QAbstractSeries * IndicatorSma::getSeriesSma() const
{
	return series;
}

void IndicatorSma::process(const std::vector<TradePrice> &v)
{
	for (auto trade = v.cbegin(); trade != v.cend(); ++trade)
	{
		series->append(
			QPointF(
				trade->getDate() * 1000,
				Calculation::Sma(trade, v.cbegin(), period)
			)
		);

		temporary.push_back(*trade);
		if (temporary.size() > period)
		{
			temporary.pop_front();
		}
	}
}

void IndicatorSma::append(const TradePrice &trade)
{
	temporary.push_back(trade);
	
	if (temporary.size() > period)
	{
		series->append(
			QPointF(
				trade.getDate() * 1000,
				Calculation::Sma(temporary.crbegin(),temporary.crend(),period)
			)
		);

		if (temporary.size() > period + 1)
		{
			temporary.pop_front();
		}
		
	}
}

void IndicatorSma::removeFirst()
{
	if (!series || !series->points().size()) return;

	series->remove(0);
	if (temporary.size() < period)
	{
		temporary.pop_front();
	}
}

void IndicatorSma::removeLast()
{
	const auto size = series->points().size();
	if (!series || !size) return;

	series->removePoints(size - 1, 1);

	temporary.pop_back();	
}

void IndicatorSma::clear()
{
	series->clear();
	temporary.clear();
}

uint IndicatorSma::getPeriod() const
{
	return period;
}

QColor IndicatorSma::getColor() const
{
	return series->color();
}

int IndicatorSma::getSize() const
{
	return series->pen().width();
}

void IndicatorSma::changePeriod(uint v)
{
	setPeriod(v);

	emit requestData();
}

void IndicatorSma::changeColor(const QColor &v)
{
	series->setColor(v);
}

void IndicatorSma::changeSize(int v)
{
	auto p = series->pen();
	p.setWidth(v);
	series->setPen(p);
}


}