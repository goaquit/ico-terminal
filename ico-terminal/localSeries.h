#pragma once

#include "TradePrice.h"

#include <QtCharts\QcandlestickSet>

#include <QtCharts\QCandlestickSeries>
#include <QtCharts\QLineSeries>

#include <vector>

using namespace QtCharts;
using namespace Entity;

inline QCandlestickSet* initCandlestickSet(const TradePrice& trade)
{
	return new QCandlestickSet(
		trade.getOpenPrice(),
		trade.getMaxPrice(),
		trade.getMinPrice(),
		trade.getClosePrice(),
		trade.getDate() * 1000
	);
}

inline void updateSeries(QLineSeries* series, const std::vector<TradePrice>& vector)
{
	series->clear();

	for (const auto trade : vector)
	{
		series->append(trade.getDate() * 1000, trade.getClosePrice());
	}
}

inline void updateSeries(QCandlestickSeries* series, const std::vector<TradePrice>& vector)
{
	series->clear();

	for (const auto& trade : vector)
	{
		series->append(initCandlestickSet(trade));
	}
}

inline void appendToSeries(QLineSeries* series, const TradePrice& trade)
{
	series->append(trade.getDate() * 1000,trade.getClosePrice());
}

inline void appendToSeries(QCandlestickSeries* series, const TradePrice& trade)
{
	series->append(initCandlestickSet(trade));
}

inline void seriesRemoveTo(QLineSeries* series, size_t distance)
{
	series->removePoints(0, static_cast<int>(distance));
}

inline void seriesRemoveTo(QCandlestickSeries* series, size_t distance)
{
	auto sets = series->sets();

	size_t counter = 0;
	for (auto itr = sets.cbegin(); counter < distance && itr != sets.cend(); ++counter, ++itr)
	{
		const auto set = *itr;
		series->remove(set);
	}
}

inline void seriesRemove(QLineSeries* series, int pos)
{
	series->removePoints(pos,1);
}

inline void seriesRemove(QCandlestickSeries* series, size_t pos)
{
	auto sets = series->sets();

	auto itr = sets.cbegin();
	size_t counter = 0;
	for (; counter < pos && itr != sets.cend(); ++counter, ++itr);
	
	const auto set = *itr;
	series->remove(set);
}