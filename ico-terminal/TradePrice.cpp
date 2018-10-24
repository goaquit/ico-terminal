#include "TradePrice.h"
#include <algorithm>

namespace Entity
{

void TradePrice::setPrice(double value)
{
	closePrice = (value > 0) ? value : 0.0;

	if (openPrice == 0.0)
	{
		openPrice = maxPrice = minPrice = closePrice;
	}

	maxPrice = std::max(maxPrice, closePrice);
	minPrice = std::min(minPrice, closePrice);
}

void TradePrice::addAmount(double value)
{
	amount += (value > 0) ? value : 0.0;
}

// запрет на перезапись
void TradePrice::setDate(time_t value)
{
	if (date != 0) return;
	date = value;
}

double TradePrice::getClosePrice() const
{
	return closePrice;
}

double TradePrice::getOpenPrice() const
{
	return openPrice;
}

double TradePrice::getMaxPrice() const
{
	return maxPrice;
}

double TradePrice::getMinPrice() const
{
	return minPrice;
}

double TradePrice::getAmount() const
{
	return amount;
}

time_t TradePrice::getDate() const
{
	return date;
}

void TradePrice::clear()
{
	closePrice = 0.0;
	openPrice = 0.0;
	maxPrice = 0.0;
	minPrice = 0.0;
	amount = 0.0;
	date = 0;
}

bool TradePrice::isEmpty()
{
	return (
		(closePrice == 0.0) &&
		(openPrice == 0.0) &&
		(maxPrice == 0.0) &&
		(minPrice == 0.0) &&
		(amount == 0.0) &&
		(date == 0)
		);
}

bool operator==(const TradePrice & lhs, const TradePrice & rhs)
{
	return
		(lhs.getDate() == rhs.getDate()) &&
		(lhs.getClosePrice() == rhs.getClosePrice()) &&
		(lhs.getOpenPrice() == rhs.getOpenPrice()) &&
		(lhs.getMaxPrice() == rhs.getMaxPrice()) &&
		(lhs.getMinPrice() == rhs.getMinPrice()) &&
		(lhs.getAmount() == rhs.getAmount());
}

bool operator!=(const TradePrice & lhs, const TradePrice & rhs)
{
	return !(lhs == rhs);
}

bool operator>(const TradePrice & lhs, const TradePrice & rhs)
{
	return lhs.getDate() > rhs.getDate();
}

bool operator<(const TradePrice & lhs, const TradePrice & rhs)
{
	return lhs.getDate() < rhs.getDate();
}

bool operator<=(const TradePrice & lhs, const TradePrice & rhs)
{
	return (lhs < rhs) || (lhs == rhs);
}

bool operator>=(const TradePrice & lhs, const TradePrice & rhs)
{
	return (lhs > rhs) || (lhs == rhs);
}

}