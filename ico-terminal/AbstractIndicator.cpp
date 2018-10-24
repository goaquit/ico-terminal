#include "AbstractIndicator.h"
#include "Chart.h"

namespace Indicators
{

AbstractIndicator::AbstractIndicator(QObject *parent) : QObject(parent) {}

AbstractIndicator::~AbstractIndicator()
{
	for (auto series : seriesList)
	{
		series->deleteLater();
	}
}

void AbstractIndicator::setChart(ChartCore::Chart *chart)
{
	if (!chart) // todo Exception
		return;

	for (const auto series : seriesList)
	{
		// remove series from another chart
		auto prevChart = series->chart();
		if (prevChart) 
		{
			prevChart->removeSeries(series);
		}

		// add new series
		chart->addSeries(series);
	}
}

void AbstractIndicator::setAlias(const QString & v)
{
	alias = v;
}


void AbstractIndicator::onReceivingNewData(const QList<Entity::TradePrice> &tradeList)
{
	for (const auto &trade : tradeList)
	{
		append(trade);
	}
}

void AbstractIndicator::onAppendData(const Entity::TradePrice &trade)
{
	append(trade);
}

void AbstractIndicator::onChangeLastData(const Entity::TradePrice &trade)
{
	removeLast();
	append(trade);
}

void AbstractIndicator::onReceivingFullData(const QList<Entity::TradePrice> &tradeList)
{
	clear();
	onReceivingNewData(tradeList);
}

}


