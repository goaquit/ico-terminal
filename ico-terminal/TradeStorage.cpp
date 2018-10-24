#include "TradePrice.h"
#include "TradeStorage.h"
#include "WrapperDb.h"

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QSqlQuery>
#include <QTimer>
#include <QTimeZone>
#include <QVariant>

QT_USE_NAMESPACE

using namespace Entity;
using namespace TerminalCore;

const int frequencyRequest = 30000;

namespace DataStorage
{


TradeStorage::TradeStorage(QObject *parent)
	: QObject(parent), mtx(new QMutex)
{
	endTime = QDateTime::currentDateTimeUtc().toTime_t();
}

TradeStorage::~TradeStorage()
{
	timer->stop();
	timer->deleteLater();
	delete mtx;

	data.clear();
}

void TradeStorage::reset()
{
	QMutexLocker lock(mtx);

	data.clear();
	lastTrade.clear();
	isFIrstLoad = true;
}

void TradeStorage::run()
{
	timer = new QTimer;
	timer->setInterval(frequencyRequest);

	connect(timer, &QTimer::timeout, this, &TradeStorage::loadData);

	timer->start();

	loadData();
}

void TradeStorage::finish()
{
	timer->stop();
	deleteLater();
}

void TradeStorage::setEndTime(uint v)
{
	userSegment = true;
	endTime = v;
}

void TradeStorage::disabledUserSegment()
{
	userSegment = false;
}

void TradeStorage::changeSegment(uint s)
{
	setSegment(s);
	reset();
	loadData();
}

void TradeStorage::changeTimeFrame(uint t)
{
	setTimeFrame(t);
	reset();
	loadData();
}

void TradeStorage::onDateInfo(uint time)
{
	for (auto ritr = data.rbegin(); ritr != data.rend(); ++ritr)
	{
		const auto tradeTime = (*ritr).getDate();
		if (time >= tradeTime - timeframe / 2 && time < tradeTime + timeframe / 2)
		{
			QString info(getTimeframeText() + ": O " + QString::number(ritr->getOpenPrice())
				+ " C " + QString::number(ritr->getClosePrice())
				+ " H " + QString::number(ritr->getMaxPrice())
				+ " L " + QString::number(ritr->getMinPrice()));
			
			emit sendDateInfo(info);
			
			break;
		}
	}
}

void TradeStorage::setExchange(uint v)
{
	exchange = v;
}

void TradeStorage::setPair(uint v)
{
	pair = v;
}

void TradeStorage::setSegment(int v)
{
	QMutexLocker lock(mtx);

	segment = v;
}

void TradeStorage::setTimeFrame(uint v)
{
	QMutexLocker lock(mtx);

	timeframe = v;
}

uint TradeStorage::getSegment() const
{
	QMutexLocker lock(mtx);

	return segment;
}

uint TradeStorage::getTimeframe() const
{
	QMutexLocker lock(mtx);

	return timeframe;
}

const QString TradeStorage::getTimeframeText() const
{
	const time_t W = 60 * 60 * 24 * 7;
	const time_t D = 60 * 60 * 24;
	const time_t H = 60 * 60;
	const time_t M = 60;

	QString result;

	if (timeframe % M == 0)
	{
		result = "M";
		result += QString::number(timeframe / M);
	}

	if (timeframe % H == 0)
	{
		result = "H";
		result += QString::number(timeframe / H);
	}

	if (timeframe % D == 0)
	{
		result = "D";
		result += QString::number(timeframe / D);
	}

	if (timeframe % W == 0)
	{
		result = "W";
		result += QString::number(timeframe / W);
	}

	return result;
}

void TradeStorage::connectIndicator(Indicators::AbstractIndicator * indicator)
{
	qRegisterMetaType<QList<TradePrice> >("QList<TradePrice>");
	qRegisterMetaType<TradePrice>("TradePrice");

	auto connect1 = connect(this, &TradeStorage::sendNewData,      indicator, &Indicators::AbstractIndicator::onReceivingNewData);
	auto connect2 = connect(this, &TradeStorage::fullData,         indicator, &Indicators::AbstractIndicator::onReceivingFullData);
	auto connect3 = connect(this, &TradeStorage::changedLastTrade, indicator, &Indicators::AbstractIndicator::onChangeLastData);
	connect(indicator, &Indicators::AbstractIndicator::requestData, [this]()
	{
		emit fullData(data);
	});

	//connect(indicator, &Indicators::AbstractIndicator::destroyed, [indicator,this,&connect1, &connect2, &connect3]()
	//{
	//	this->disconnect(connect1);
	//	this->disconnect(connect2);
	//	this->disconnect(connect3);
	//});
}

void TradeStorage::fullDataRequest()
{
	emit fullData(data);
}

void TradeStorage::loadData()
{
	if (!exchange || !pair) return;

	QMutexLocker lock(mtx);

	auto endDate = (userSegment) ? QDateTime::fromTime_t(endTime).toUTC() : QDateTime::currentDateTimeUtc();
	const int localSegment = (isFIrstLoad) ? segment + (timeframe * 50) : timeframe;
	const auto startDate = endDate.addSecs(-localSegment);

	const auto sql = "select open,close,max,min,amount,date \
from select_trades_timeframe('" + startDate.toString(Qt::ISODate) + "',\
'" + endDate.toString(Qt::ISODate) + "'," + QString::number(exchange) + ",\
" + QString::number(pair) + "," + QString::number(timeframe) + ");";

	auto query = WrapperDb::instance()->select(sql);

	QList<TradePrice> tmpData;

	while (query->next())
	{
		const auto open   = query->value(0).toDouble();
		const auto close  = query->value(1).toDouble();
		const auto max    = query->value(2).toDouble();
		const auto min    = query->value(3).toDouble();
		const auto amount = query->value(4).toDouble();
		auto tradeDate    = query->value(5).toDateTime();

		tradeDate.setTimeSpec(Qt::UTC);		

		TradePrice trade;

		trade.setPrice(open);
		trade.setPrice(max);
		trade.setPrice(min);
		trade.setPrice(close);
		trade.addAmount(amount);
		trade.setDate(tradeDate.toTime_t());		
		

		if (trade <= lastTrade) continue;

		// lasttrade changed
		if (trade != lastTrade && trade.getDate() == lastTrade.getDate() && data.size())
		{
			data.pop_back();
			data.push_back(trade);

			emit changedLastTrade(trade);

			lastTrade = trade;
		}
		else
		{
			tmpData.push_back(trade);
		}

		
	}

	if (tmpData.size())
	{
		emit sendNewData(tmpData);

		data.append(tmpData);
		lastTrade = *data.rbegin();

		uint size = data.size();
		if (isFIrstLoad && (segment / timeframe) > size)
		{
			segment = timeframe * data.size();
		}

		isFIrstLoad = false;

		if (segment / timeframe <= size) return;

		auto itrTo = data.begin();
		for (;itrTo != data.end() && itrTo->getDate() < startDate.toTime_t(); ++itrTo);
		data.erase(data.begin(), itrTo);
	}
}


}