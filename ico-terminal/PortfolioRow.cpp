#include "PortfolioRow.h"

PortfolioRow::PortfolioRow(const QString & e, const QString & s, double v)
	: exchange(e), symbol(s), volume(v)
{
}

void PortfolioRow::setExchagne(const QString & v){	exchange = v;}
void PortfolioRow::setSymbol(const QString & v){	symbol = v;}
void PortfolioRow::setVolume(double v){	volume = v;}
void PortfolioRow::setPrice(double v){	price = v;}
void PortfolioRow::setTotal(double v){	total = v;}

bool operator==(const PortfolioRow & rhs, const PortfolioRow & lhs)
{
	auto result = !rhs.getExchange().trimmed().compare(lhs.getExchange().trimmed(),Qt::CaseInsensitive) &&
		!rhs.getSymbol().trimmed().compare(lhs.getSymbol().trimmed(),Qt::CaseInsensitive);

	return result;
}

bool operator!=(const PortfolioRow & rhs, const PortfolioRow & lhs)
{
	return !(rhs == lhs);
}
