#include "TradeLoader.h"
#include "WrapperDb.h"

#include <QDateTime>
#include <QVariant>

#include <QDebug>
#include <QSqlQuery>

static inline  QString DetectTimeFrameTable(time_t timeFrame)
{
	QString table = "trades_timeframe_";

	const auto M15 = 60  * 15;
	const auto M30 = M15 * 2;
	const auto H1  = M30 * 2;
	const auto H4  = H1  * 4;
	const auto H12 = H4  * 3;
	const auto D1  = H12 * 2;

	if (timeFrame >= D1)
	{
		table += "D1";
	}
	else if (timeFrame >= H12)
	{
		table += "H12";
	}
	else if (timeFrame >= H4)
	{
		table += "H4";
	}
	else if (timeFrame >= H1)
	{
		table += "H1";
	}
	else if (timeFrame >= M30)
	{
		table += "M30";
	}
	else 
	{
		table += "M15";
	}


	return table;
}

using namespace TerminalCore;


namespace Terminal
{
	QList<TradePrice> TradeLoader::loadData(time_t start, time_t end, time_t timeFrame)
	{
		QList<TradePrice> list;
		
		if (!exchangeId || !pair) return list;

		auto startDate = QDateTime::fromTime_t(start).toUTC();
		const auto endDate = QDateTime::fromTime_t(end).toUTC();

		const QString sql = "select open,close,max,min,amount,date from \"" + DetectTimeFrameTable(timeFrame) + "\" \
		where exchange = '"+QString::number(exchangeId)+"' and  pair = '"+QString::number(pair)+"' \
		and date between '" + startDate.toString(Qt::ISODate) + "' and '" + endDate.toString(Qt::ISODate) + "' \
		order by date asc";

		auto query = WrapperDb::instance()->select(sql);

		while (query->next())
		{
			TradePrice trade;
			
			const auto open      = query->value(0).toDouble();
			const auto close     = query->value(1).toDouble();
			const auto max       = query->value(2).toDouble();
			const auto min       = query->value(3).toDouble();
			const auto amount    = query->value(4).toDouble();
			      auto tradeDate = query->value(5).toDateTime();

			trade.setPrice(open);
			trade.setPrice(max);
			trade.setPrice(min);
			trade.setPrice(close);
			trade.addAmount(amount);

			
			tradeDate.setTimeSpec(Qt::UTC);
			trade.setDate(tradeDate.toTime_t());

			list.append(trade);
		}


		delete query;

		return list;
	}
}