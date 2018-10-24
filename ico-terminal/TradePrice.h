#ifndef TRADE_PRICE_H
#define TRADE_PRICE_H

#include <ctime>

namespace Entity
{
	class TradePrice
	{
	public:
		enum Price 
		{
			Open,
			Close,
			High,
			Low
		};
		
		void setPrice(double);
		void addAmount(double);
		void setDate(time_t);

		double getClosePrice() const;
		double getOpenPrice() const;
		double getMaxPrice() const;
		double getMinPrice() const;

		double getAmount() const;

		time_t getDate() const;

		void clear();
		bool isEmpty();

	private:
		double closePrice = 0.0;
		double openPrice  = 0.0;
		double maxPrice   = 0.0;
		double minPrice   = 0.0;
		double amount     = 0.0;

		time_t date  = 0;

	};

	bool operator==(const TradePrice& lhs, const TradePrice& rhs);
	bool operator!=(const TradePrice& lhs, const TradePrice& rhs);
	bool operator >(const TradePrice &lhs, const TradePrice &rhs);
	bool operator <(const TradePrice &lhs, const TradePrice &rhs);
	bool operator<=(const TradePrice &lhs, const TradePrice &rhs);
	bool operator>=(const TradePrice &lhs, const TradePrice &rhs);
}


#endif // !TRADE_PRICE_H