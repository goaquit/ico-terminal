#pragma once
#include <deque>
#include <vector>

#include "TradePrice.h"

using vectorItr  = std::vector<Entity::TradePrice>::const_iterator;
using dequeItr   = std::deque<Entity::TradePrice>::const_iterator;
using deuqeRitr  = std::deque<Entity::TradePrice>::const_reverse_iterator;
using dequeTrade = std::deque<Entity::TradePrice>;
using TypePrice           = Entity::TradePrice::Price;

class Calculation
{
public:
	static double Sma(const std::vector<double> &input, unsigned period);
	static double Sma(const std::deque<double> &input, unsigned period);
	static double Sma(const vectorItr start, const vectorItr end, const size_t period, TypePrice type = TypePrice::Close);
	static double Sma(const dequeItr start, const dequeItr end, const size_t period, TypePrice type = TypePrice::Close);
	static double Sma(const deuqeRitr start, const deuqeRitr end, const size_t period, TypePrice type = TypePrice::Close);

	static double FastStochastic(const dequeTrade &trades, deuqeRitr &start, const size_t period);

	static double Sar(double sar, double ep, double af, bool isLong = true);

	static double StandardDeviation(const deuqeRitr start, const deuqeRitr end, double meanValue, unsigned period);
};

