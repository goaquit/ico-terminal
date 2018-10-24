#pragma once
#include <QString>

class PortfolioRow
{
public:
	PortfolioRow() = default;
	PortfolioRow(const QString &exchange, const QString &symbol, double volume);

	void setExchagne(const QString &exchange);
	void setSymbol(const QString &symbol);
	void setVolume(double volume);
	void setPrice(double price);
	void setTotal(double total);

	inline QString getExchange() const {return exchange;}
	inline QString getSymbol()   const {return symbol;  }
	inline double  getVolume()   const {return volume;	}
	inline double  getPrice()    const {return price;	}
	inline double  getTotal()    const {return total;	}

private:
	QString exchange = "";
	QString symbol   = "";
	double  volume   = 0.0;
	double  price    = 0.0;
	double  total    = 0.0;
};

bool operator==(const PortfolioRow &rhs, const PortfolioRow &lhs);
bool operator!=(const PortfolioRow &rhs, const PortfolioRow &lhs);