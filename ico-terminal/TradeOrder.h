#pragma once
class TradeOrder
{
public:

	enum TypePosition
	{
		Long,
		Short
	};

	void buy(const double&);
	void sell(const double&);

	void buyLimit(const double&);
	void sellLimit(const double&);

	void close();

	void setStopLoss(const double&);
	void setTakeProfit(const double &);

private:
	TypePosition type;
	double openPrice  = 0.0;
	double stopLoss   = 0.0;
	double takeProfit = 0.0;
};

