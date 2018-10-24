#include "Position.h"
#include "TradeApiWex.h"
#include <QTimer>
#include <QVector>
#include <QCryptographicHash>

#include <cmath>
#include <limits>

bool is_equal(double x, double y) {
	return std::fabs(x - y) < std::numeric_limits<double>::epsilon();
}

Position::Position(const Position &copy)
{
	*this = copy;
}

Position & Position::operator=(const Position & copy)
{
	price		= copy.price;
	stopLoss	= copy.stopLoss;
	takeProfit	= copy.takeProfit;
	volume		= copy.volume;
	volumeCur	= copy.volumeCur;
	dateOpen	= copy.dateOpen;
	type		= copy.type;
	pair		= copy.pair;
	exchange    = copy.exchange;
	status      = copy.status;
	orders		= copy.orders;
	apiPair     = copy.apiPair;

	return *this;
}

Position::Position(const double & p, const double & v, const time_t & d, const Type & t, const QString & pr, const double & sl, const double & tp)
	: price(p), volume(v), type(t), pair(pr), dateOpen(d), stopLoss(sl), takeProfit(tp)
{
}


/////////////////////////////////////////////////////////////////////////////
// setters
void Position::setPrice(const double &v)	 {	price = v;		}
void Position::setDateOpen(const time_t &v)	 {	dateOpen = v;	}
void Position::setStopLoss(const double &v)	 {	stopLoss = v;	}
void Position::setTakeProfit(const double &v){	takeProfit = v;	}
void Position::setVolume(const double & v)	 {	volume = v;		}
void Position::setVolCur(const double &v)	 {	volumeCur = v;	}
void Position::setType(const Type &v)		 {	type = v;		}
void Position::setStatus(const Status &v)	 {	status = v;		}
void Position::setPair(const QString &v)	 {	pair = v;		}
void Position::setExchange(const QString &v) {	exchange = v;   }
void Position::setApiPair(const QString &v)  {	apiPair = v;	}

//void Position::setOrders(const std::vector<TradeOrderInfo*>&o) {	orders = o;}
void Position::setOrders(const QVector<TradeOrderInfo> &o){ orders = o;}
void Position::setTradeApi(const TradeApiWex &v)		 
{	
	trade.disconnect();

	trade = v;	

	auto c1 = QObject::connect(&trade, &TradeApiWex::error, this, &Position::onError);

	auto c2 = QObject::connect(&trade, &TradeApiWex::isSuccess, this, &Position::isSuccess);

	auto c3 = QObject::connect(&trade, &TradeApiWex::newOrder, [this](const TradeOrderInfo &order)
	{
		appenOrder(order);

		emit requestToSave();
	});

	auto c4 = QObject::connect(&trade, &TradeApiWex::canceledOrder, [this](const TradeOrderInfo &order)
	{
		removeOrder(order);
		if (!orders.size())
		{
			emit readyToDelete();
		}
	});

	auto c5 = QObject::connect(&trade, &TradeApiWex::volumeRecieved, this, &Position::onVolumeRecieved);

	QObject::connect(this, &Position::check, this, &Position::onCheck);

	connect(this, &Position::destroyed, [this, c1, c2, c3, c4, c5]()
	{
		this->disconnect(c1); this->disconnect(c2); this->disconnect(c3); this->disconnect(c4); this->disconnect(c5); 
	});

	for (auto el = orders.rbegin(); el != orders.rend(); ++el)
	{
		connectToOrder(&*el);
	}
}

//
/////////////////////////////////////////////////////////////////////////////

void Position::connectToOrder(TradeOrderInfo * order)
{
	auto timer = new QTimer;

	connect(timer, &QTimer::timeout, [this, order](){trade.orderInfo(order->getId());});
	connect(order, &TradeOrderInfo::destroyed, [timer]()
	{
		if (!timer) return;
		timer->stop();
		timer->deleteLater();
	});

	auto connectRecieved = connect(&trade, &TradeApiWex::recievedOrderInfo, this, &Position::onRecievedOrderInfo);

	connect(this, &Position::destroyed, [this, connectRecieved, timer]()
	{
		this->disconnect(connectRecieved);
	});

	timer->setInterval(5000);
	timer->start();
}

void Position::appenOrder(const TradeOrderInfo & order)
{
	auto isFind = !orders.size();

	if (!isFind)
	{
		for (const auto el : orders)
		{
			isFind = el.getId() == order.getId();
			if (isFind)
			{
				return;
			}
		}
	}

	orders.push_back(order);

	qDebug() << "New order\tID " << QString::number(order.getId())
		<< " " << QString::number(order.getStartAmount())
		<< " / " << QString::number(order.getAmount())
		<< " " << pair;

	for (auto el = orders.rbegin(); el != orders.rend(); ++el)
	{
		if (*el == order) connectToOrder(&*el);
	}

	if (status == Position::New) status = Opening;
}

void Position::removeOrder(const TradeOrderInfo & order)
{
	orders.removeOne(order);

	emit requestToSave();
}

void Position::cancelAllOrders()
{
	if (!orders.size())
	{
		emit readyToDelete();
		return;
	}
	for (const auto &order : orders)
	{
		trade.cancelOrder(order.getId());
	}
}

QByteArray Position::getHash() const
{
	return QCryptographicHash::hash(
		(QString::number(price) + "##" + QString::number(dateOpen)).toUtf8(), QCryptographicHash::Md5
	);
}

void Position::open(double p)
{
	if (!trade.isAvailable())
	{
		qDebug() << "TradeApi not available";
		return;
	}

	status = New;

	emit requestToSave();

	const auto priceOpen = (p == 0.0) ? price : p;

	trade.openOrder(priceOpen, volume, type == Long ? TradeApiCore::Buy : TradeApiCore::Sell, apiPair);
}

void Position::close(double closePrice)
{
	emit sendMessage(
		"Closing position " + QString((type == Position::Long) ? "Long " : "Short ")
		+ " price: " + QString::number(price) + " volume: " + QString::number(volumeCur) + " " + pair + ". Price close: " + QString::number(closePrice)
	);

	if (volumeCur <= 0.0)
	{
		emit removing(this);
		return;
	}

	if (!trade.isAvailable())
	{
		qDebug() << "TradeApi not available";
		return;
	}

	status = Closing;

	emit requestToSave();

	cancelAllOrders();

	trade.openOrder(closePrice, volumeCur, type != Long ? TradeApiCore::Buy : TradeApiCore::Sell, apiPair);
}

void Position::onError(const QString &msg)
{
	qDebug() << msg;
	
	emit removing(this);
	emit error(msg);
}

void Position::onVolumeRecieved(double vc)
{
	switch (status)
	{
	case Position::New:
		status = Opening;

	case Position::Opening:

		volumeCur += vc;

		if (is_equal(volume,volumeCur))
		{
			status = Open;
		}

		emit requestToSave();
		emit changed(this);

		break;
	case Position::Open:

		// :(

		break;
	case Position::Closing:

		volumeCur -= vc;

		if (is_equal( volumeCur,0.0))
		{
			emit removing(this);
			emit readyToDelete();
		}
		else
		{
			emit requestToSave();
			emit changed(this);
		}

		break;
	case Position::Close:

		emit removing(this);

		break;
	}

	
}

void Position::onCheck(double newPrice)
{
	if(status == New || (stopLoss == 0.0 && takeProfit == 0.0)) return;

	switch (type)
	{
		case Position::Long:

			// add 5% to price (Market order in Wex)
			if (stopLoss > 0.0 && newPrice < stopLoss)     
				close(newPrice);
			else if (takeProfit > 0.0 && newPrice > takeProfit) 
				close(newPrice);

		break;
		case Position::Short:

			// add 5% to price (Market order in Wex)
			if (stopLoss > 0.0 && newPrice > stopLoss)     
				close(newPrice);
			else if (takeProfit > 0.0 && newPrice < takeProfit) 
				close(newPrice);
		break;
	}
}


//todo: refactoring
void Position::onRecievedOrderInfo(const TradeOrderInfo & order)
{
	TradeOrderInfo *prevOrder = nullptr;

	for (auto &el : orders)
	{
		if (el == order)
		{
			prevOrder = &el;

			break;
		}
	}

	if (!prevOrder) return;

	const auto diff = std::fabs( (prevOrder->getStartAmount() - prevOrder->getAmount()) - (order.getStartAmount() - order.getAmount()));

	switch (type)
	{
	case Position::Long:

		volumeCur += (order.getType() == TradeOrderInfo::Bye) ? diff : -diff;
		if (volumeCur < 0.0) volumeCur = 0.0;

		break;
	case Position::Short:

		volumeCur += (order.getType() == TradeOrderInfo::Sell) ? diff : -diff;
		if (volumeCur < 0.0) volumeCur = 0.0;

		break;
	}

	*prevOrder = order;
	auto statusOrder = order.getStatus();
	switch (statusOrder)
	{
	case TradeOrderInfo::Executed:
	case TradeOrderInfo::Canceled:
	case TradeOrderInfo::Partial:
		removeOrder(*prevOrder);
	case TradeOrderInfo::Active:
		
		switch (status)
		{
			case Position::New:	    status = Opening; break;
			case Position::Opening: status = is_equal(volume, volumeCur) ? Open  : Opening; break;
			case Position::Closing: status = is_equal(0.0, volumeCur)    ? Close : Closing; break;
		}

		if (status == Close || (status == Opening && !orders.size()))
		{
			emit removing(this);
			emit readyToDelete();
		}

		break;
	}

	emit requestToSave();
	emit changed(this);
}

//////////////////////////////////////////////////////////////////////
// serialization
QDataStream & operator<<(QDataStream & out, const Position::Type & t)
{
	out << quint8(t);
	return out;
}
QDataStream & operator>>(QDataStream & in, Position::Type & t)
{
	quint8 type;
	in >> type;
	t = Position::Type(type);
	return in;
}
QDataStream & operator<<(QDataStream & out, const Position::Status & s)
{
	out << quint8(s);
	return out;
}
QDataStream & operator>>(QDataStream & in, Position::Status & s)
{
	quint8 status;
	in >> status;
	s = Position::Status(status);
	return in;
}
QDataStream & operator<<(QDataStream & out, const Position & position)
{
	out << position.getPrice() << position.getDateOpen() << position.getVolume()	<< position.getVolCur()		 << position.getPair()	 << position.getExchange()
		<< position.getType()  << position.getStatus()	 << position.getStopLoss()	<< position.getTakeProfite() << position.getOrders() << position.getApiPair()
		<< QByteArray(64, 0);

	return out;
}
QDataStream & operator>>(QDataStream & in, Position & position)
{
	double  price;
	double  volume;
	double  volumeCur;
	double  stopLoss;
	double  takeProfit;
	time_t  dateOpen;
	QString pair;
	QString exchange;
	QString apiPair;

	Position::Type	 type;
	Position::Status status;

	auto buffer(QByteArray(64, 0));

	QVector<TradeOrderInfo> orders;

	in >> price >> dateOpen >> volume >> volumeCur >> pair >> exchange >> type >> status >> stopLoss >> takeProfit >> orders >> apiPair >> buffer;

	position.setPrice(price);	position.setDateOpen(dateOpen); position.setVolume(volume);			position.setVolCur(volumeCur); position.setPair(pair);
	position.setType(type);		position.setStopLoss(stopLoss); position.setTakeProfit(takeProfit); position.setOrders(orders);
	position.setStatus(status); position.setExchange(exchange); position.setApiPair(apiPair);

	return in;
}
bool operator==(const Position & lhs, const Position & rhs)
{
	return lhs.getPrice() == rhs.getPrice() && lhs.getDateOpen() == rhs.getDateOpen();
}
