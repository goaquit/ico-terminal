#include "Calculation.h"

#include <algorithm>
#include <cmath>

using namespace Entity;

double Calculation::Sma(const std::vector<double>& input, unsigned period)
{
	if (!period) return (input.size()) ? *input.rbegin() : 0;

	double sum = 0.0;
	auto n = period;
	for (auto itr = input.crbegin(); itr != input.crend() && n > 0; ++itr, --n)
	{
		sum += *itr;
	}

	return sum / period;
}

double Calculation::Sma(const std::deque<double>& input, unsigned period)
{
	if (!period) return (input.size() > 0) ? *input.rbegin() : 0;

	double sum = 0.0;
	auto n = period;
	for (auto itr = input.crbegin(); itr != input.crend() && n > 0; ++itr, --n)
	{
		sum += *itr;
	}

	return sum / period;
}

double Calculation::Sma(const vectorItr  start, const vectorItr  end, const size_t period, TypePrice type)
{
	if (!period)
	{
		double result = 0.0;
		switch (type)
		{
			case TypePrice::Open:  result = start->getOpenPrice();  break;
			case TypePrice::Close: result = start->getClosePrice(); break;
			case TypePrice::High:  result = start->getMaxPrice();   break;
			case TypePrice::Low:   result = start->getMinPrice();   break;
		}

		return result;
	}

	auto sum     = 0.0;
	auto counter = period;

	for (auto itr = start; itr != end && !counter; --itr, --counter)
	{
		switch (type)
		{
			case TypePrice::Open:  sum += itr->getOpenPrice();  break;
			case TypePrice::Close: sum += itr->getClosePrice(); break;
			case TypePrice::High:  sum += itr->getMaxPrice();   break;
			case TypePrice::Low:   sum += itr->getMinPrice();   break;
		}
	}

	return sum / period;
}

double Calculation::Sma(const dequeItr start, const dequeItr end, const size_t period, TypePrice type)
{
	if (!period)
	{
		double result = 0.0;
		switch (type)
		{
		case TypePrice::Open:  result = start->getOpenPrice();  break;
		case TypePrice::Close: result = start->getClosePrice(); break;
		case TypePrice::High:  result = start->getMaxPrice();   break;
		case TypePrice::Low:   result = start->getMinPrice();   break;
		}

		return result;
	}

	auto sum = 0.0;
	auto counter = period;

	for (auto itr = start; itr != end && counter; --itr, --counter)
	{
		switch (type)
		{
		case TypePrice::Open:  sum += itr->getOpenPrice();  break;
		case TypePrice::Close: sum += itr->getClosePrice(); break;
		case TypePrice::High:  sum += itr->getMaxPrice();   break;
		case TypePrice::Low:   sum += itr->getMinPrice();   break;
		}
	}

	return sum / period;
}

double Calculation::Sma(const deuqeRitr start, const deuqeRitr end, const size_t period, TypePrice type)
{
	if (!period)
	{
		double result = 0.0;
		switch (type)
		{
		case TypePrice::Open:  result = start->getOpenPrice();  break;
		case TypePrice::Close: result = start->getClosePrice(); break;
		case TypePrice::High:  result = start->getMaxPrice();   break;
		case TypePrice::Low:   result = start->getMinPrice();   break;
		}

		return result;
	}

	auto sum = 0.0;
	auto counter = period;

	for (auto itr = start; itr != end && counter; ++itr, --counter)
	{
		switch (type)
		{
		case TypePrice::Open:  sum += itr->getOpenPrice();  break;
		case TypePrice::Close: sum += itr->getClosePrice(); break;
		case TypePrice::High:  sum += itr->getMaxPrice();   break;
		case TypePrice::Low:   sum += itr->getMinPrice();   break;
		}
	}

	return sum / period;
}

double Calculation::FastStochastic(const dequeTrade & trades, deuqeRitr & start, const size_t period)
{
	double result = 0.0;

	if (!trades.size() || start == trades.crend()) return result;

	      double low   = start->getMinPrice();
	      double high  = start->getMaxPrice();
	const double close = start->getClosePrice();

	auto counter = 0;
	for (; start != trades.crend() && counter != period; ++start, ++counter)
	{
		low  = std::min(low, start->getMinPrice());
		high = std::max(high, start->getMaxPrice());
	}

	if (high == low) return result;

	/*
	%K = 100[(C – L14)/(H14 – L14)]

	C = the most recent closing price 
	L14 = the low of the 14 previous trading sessions 
	H14 = the highest price traded during the same 14-day period.
	*/

	result = 100 * ( (close - low) / (high - low) ); 

	return result;
}

double Calculation::Sar(double sar, double ep, double af, bool isLong)
{
	double diff = std::fabs(ep - sar);
	diff = (isLong) ? diff : -diff;

	return sar + (diff * af);
}

double Calculation::StandardDeviation(const deuqeRitr start, const deuqeRitr end, double meanValue, unsigned period)
{
	if (period <= 1) return 0.0;

	auto sum = 0.0;
	auto counter = period;
	for (auto itr = start; itr != end && period; ++itr, --counter)
	{
		sum += std::pow(itr->getClosePrice() - meanValue,2);
	}

	return std::sqrt(sum / (period - 1));
}

