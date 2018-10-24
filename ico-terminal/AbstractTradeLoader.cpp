#include "AbstractTradeLoader.h"
#include "TradePrice.h"

#include <ctime>
#include <random>

#include <QDateTime>

namespace Terminal
{


void AbstractTradeLoader::run(quint64 start, quint64 end, quint64 timeFrame)
{
	emit sendDate(loadData(start, end, timeFrame));
}

void AbstractTradeLoader::setExchange(uint32_t value)
{
	exchangeId = value;
}

void AbstractTradeLoader::setPair(uint32_t value)
{
	pair = value;
}

void AbstractTradeLoader::requestData() const
{
}


}