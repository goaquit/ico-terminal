#include "Series.h"

#include "localSeries.h"

#include <algorithm>
#include <iterator>

#include <QtCharts\QCandlestickSeries>
#include <QtCharts\QLineSeries>



namespace ChartCore
{
Series::Series(QObject *parent)
	: QObject(parent),
	line(new QLineSeries(this)),
	candlestick(new QCandlestickSeries(this)),
	amount(new QLineSeries(this)){}
	
	void Series::init(ChartSeriesType t)
	{
		candlestick->setIncreasingColor(Qt::green);
		candlestick->setDecreasingColor(Qt::red);
		candlestick->setBodyOutlineVisible(false);
		candlestick->setPen(QPen(Qt::lightGray, 1));

		amount->setPen(QPen(Qt::darkYellow, 2));

		change(t);
	}

	void Series::change(ChartSeriesType t)
	{
		type = t;

		clear();

		switch (type)
		{
		case ChartCore::Line:
			updateSeries(line, data);
			main = line;
			break;
		case ChartCore::Candlestick:
			updateSeries(candlestick,data);
			main = candlestick;
			break;
		default:
			break;
		}
	}

	void Series::append(const TradePrice& tradePrice)
	{
		data.push_back(tradePrice);

		lastTradePrice = tradePrice;

		switch (type)
		{
		case ChartCore::Line:
			appendToSeries(line, tradePrice);
			break;
		case ChartCore::Candlestick:
			appendToSeries(candlestick, tradePrice);
			break;
		default:
			break;
		}

		amount->append(tradePrice.getDate() * 1000, tradePrice.getAmount());		
	}

	void Series::clear()
	{
		data.clear();

		line->clear();
		candlestick->clear();
		
		amount->clear();

		lastTradePrice.clear();

	}

	void Series::removeFirst()
	{
		if (!data.size()) return;
		data.erase(data.begin());

		switch (type)
		{
			case ChartCore::Line:        seriesRemoveTo(line, 1);        break;
			case ChartCore::Candlestick: seriesRemoveTo(candlestick, 1); break;
		}
	}

	void Series::removeLast()
	{
		if (data.size() < 2) return;
		const auto lastPos = static_cast<int>(data.size() - 1);
		data.pop_back();

		lastTradePrice = *data.rbegin();

		switch (type)
		{
			case ChartCore::Line:        seriesRemove(line, lastPos);        break;
			case ChartCore::Candlestick: seriesRemove(candlestick, lastPos); break;
		}

		seriesRemove(amount, lastPos);
	}

	void Series::removeTo(const TradePrice& trade)
	{
		const auto itr = std::find(data.begin(), data.end(), trade);
		if (!data.size() || (itr == data.end()) )
			return;

		const size_t distance = std::distance(data.begin(), itr);

		switch (type)
		{
		case ChartCore::Line:
			seriesRemoveTo(line, distance);
			break;
		case ChartCore::Candlestick:
			seriesRemoveTo(candlestick, distance);
			break;
		}
	}

	void Series::updateLastTrade(const TradePrice& trade)
	{
		removeLast();
		append(trade);
	}

	QAbstractSeries * Series::getSeries() const
	{
		return main;
	}

	QAbstractSeries * Series::getAmount() const
	{
		return amount;
	}

	void Series::processRange(time_t start, time_t end)
	{
		if (!data.size()) return;

		const auto lastTrade = data.crbegin();
		max = lastTrade->getMaxPrice();
		min = lastTrade->getMinPrice();

		maxAmount = 0;

		for (const auto& trade : data)
		{
			const auto date = trade.getDate();
			if (date < start) continue;
			if (date > end) break;

			maxAmount = std::max(maxAmount, trade.getAmount());

			switch (type)
			{
				case ChartCore::Line:
				{
					const auto price = trade.getClosePrice();
					max = std::max(max, price);
					min = std::min(min, price);
				} break;
				case ChartCore::Candlestick:
				
					max = std::max(max, trade.getMaxPrice());
					min = std::min(min, trade.getMinPrice());
				
				break;
			}

			
		}
	}

	double Series::getMax() const
	{
		return max;
	}

	double Series::getMin() const
	{
		return min;
	}
	double Series::getMaxAmount() const
	{
		return maxAmount;
	}
	TradePrice Series::getLastTrade() const
	{
		return lastTradePrice;
	}
	size_t Series::size() const
	{
		return data.size();
	}
	
}