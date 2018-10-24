#include "TradeOrderInfo.h"
#include <QDataStream>
#include <QString>

TradeOrderInfo::TradeOrderInfo(uint i, QObject *parent) :  id(i), QObject(parent)
{
}

TradeOrderInfo::TradeOrderInfo(uint i, const QString & p, double sa, double a, double r, time_t c, Type t, Status s, QObject *parent) :
	id(i), pair(p), startAmount(sa), amount(a), rate(r), createTimestamp(c), type(t), status(s), QObject(parent)
{
}

TradeOrderInfo::TradeOrderInfo(const TradeOrderInfo & copy)
{
	id = copy.id;
	pair = copy.pair;
	startAmount = copy.startAmount;
	amount = copy.amount;
	rate = copy.rate;
	createTimestamp = copy.createTimestamp;
	type = copy.type;
	status = copy.status;
}

TradeOrderInfo & TradeOrderInfo::operator=(const TradeOrderInfo & copy)
{
	id				= copy.id;
	pair			= copy.pair;
	startAmount		= copy.startAmount;
	amount			= copy.amount;
	rate			= copy.rate;
	createTimestamp = copy.createTimestamp;
	type			= copy.type;
	status			= copy.status;

	return *this;
}

void TradeOrderInfo::setId(const uint &v)				{id = v;			 }
void TradeOrderInfo::setPair(const QString &v)			{pair = v;			 }
void TradeOrderInfo::setStartAmount(const double &v)	{startAmount = v; amount = v;}
void TradeOrderInfo::setAmount(const double &v)			{amount = v;		 }
void TradeOrderInfo::setRate(const double &v)			{rate = v;			 }
void TradeOrderInfo::setCreateTimestamp(const time_t &v){createTimestamp = v;}
void TradeOrderInfo::setType(const Type &v)				{type = v;			 }
void TradeOrderInfo::setStatus(const Status &v)			{status = v;		 }

QDataStream & operator<<(QDataStream & out, const TradeOrderInfo::Type & v)
{
	out << static_cast<uint>(v);
	return out;
}

QDataStream & operator>>(QDataStream & in, TradeOrderInfo::Type & v)
{
	uint type;
	in >> type;
	v = TradeOrderInfo::Type(type);

	return in;
}

QDataStream & operator<<(QDataStream & out, const TradeOrderInfo::Status & v)
{
	out << static_cast<uint>(v);
	return out;
}

QDataStream & operator>>(QDataStream & in, TradeOrderInfo::Status & v)
{
	uint status;
	in >> status;
	v = TradeOrderInfo::Status(status);

	return in;
}

QDataStream & operator<<(QDataStream & out, const TradeOrderInfo & o)
{
	out << o.getId() << o.getPair()	<< o.getStartAmount() << o.getAmount() << o.getRate() << o.getCreateTimestamp() << o.getType() << o.getStatus() << QByteArray(64, 0);;
	return out;
}

QDataStream & operator>>(QDataStream & in, TradeOrderInfo & order)
{
	uint					id				= 0;
	QString					pair			= "";
	double					startAmount		= 0.0;
	double					amount			= 0.0;
	double					rate			= 0.0;
	time_t					createTimestamp	= 0;
	TradeOrderInfo::Type	type			= TradeOrderInfo::None;
	TradeOrderInfo::Status  status			= TradeOrderInfo::Active;

	auto buffer(QByteArray(64, 0));

	in >> id >> pair >> startAmount >> amount >> rate >> createTimestamp >> type >> status >> buffer;
	order = TradeOrderInfo(id,pair,startAmount,amount,rate,createTimestamp,type,status);

	return in;
}

bool operator==(const TradeOrderInfo & lhs, const TradeOrderInfo & rhs)
{
	return lhs.getId() == rhs.getId();
}

bool operator!=(const TradeOrderInfo & lhs, const TradeOrderInfo & rhs)
{
	return !(lhs == rhs);
}
