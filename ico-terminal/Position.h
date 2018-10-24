#pragma once
#include <ctime>
#include <QObject>
#include <QDataStream>
#include "TradeOrderInfo.h"
#include "TradeApiWex.h"


class Position : public QObject
{
	Q_OBJECT
public:
	enum Type : quint8
	{
		Long,
		Short
	};

	enum Status : quint8
	{
		New,
		Opening,
		Open,
		Closing,
		Close
	};

	Position() = default;
	Position(const Position &);
	Position & operator=(const Position &copy);
	Position(
		const double  &price,
		const double  &volume,
		const time_t  &dateOpen,
		const Type    &type,
		const QString &pair,
		const double  &stopLoss = 0.0,
		const double  &takeProfit = 0.0
	);

	void setPrice(const double &);
	void setDateOpen(const time_t &);
	void setStopLoss(const double &);
	void setTakeProfit(const double &);
	//void setOrders(const std::vector<TradeOrderInfo*> &);
	void setOrders(const QVector<TradeOrderInfo> &);
	void setVolume(const double &);
	void setVolCur(const double &);
	void setType(const Type &);
	void setStatus(const Status &);
	void setPair(const QString&);
	void setExchange(const QString &);
	void setTradeApi(const TradeApiWex &);
	void setApiPair(const QString &);

	double  getPrice()		 const { return price;		}
	time_t  getDateOpen()	 const { return dateOpen;	}
	double  getStopLoss()	 const { return stopLoss;	}
	double  getTakeProfite() const { return takeProfit;	}
	double  getVolume()		 const { return volume;		}
	double  getVolCur()      const { return volumeCur;  }
	Type    getType()        const { return type;		}
	Status  getStatus()      const { return status;		}
	QString getPair()		 const { return pair;		}
	QString getExchange()    const { return exchange;	}
	QString getApiPair()     const { return apiPair; 	}

	//std::vector<TradeOrderInfo*> getOrders() const { return orders; }
	QVector<TradeOrderInfo> getOrders() const {		return orders;	}

	void appenOrder(const TradeOrderInfo &order);
	void removeOrder(const TradeOrderInfo &order);

	void cancelAllOrders();

	QByteArray getHash() const;

public slots:
	void onRecievedOrderInfo(const TradeOrderInfo &order);
	void onVolumeRecieved(double volume);
	void onCheck(double price);
	// if price == 0.0 then use position price
	void open(double price = 0.0);
	void close(double price);
	void onError(const QString &);

signals:
	void requestToSave();
	void changed(const Position *);
	void error(const QString &);
	void isSuccess();
	void check(double price);
	void closing();
	void removing(Position *);
	void readyToDelete();
	void sendMessage(const QString &);

private:
	double  price		= 0.0;
	double  stopLoss	= 0.0;
	double  takeProfit	= 0.0;
	double  volume		= 0.0;
	double  volumeCur   = 0.0;
	time_t  dateOpen	= 0;
	Type    type		= Long;
	Status  status      = New;
	QString pair		= "";
	QString exchange    = "";
	QString apiPair     = "";

	QVector<TradeOrderInfo> orders;

	TradeApiWex trade;

	void connectToOrder(TradeOrderInfo *order);
};

QDataStream &operator<<	(	   QDataStream &out, const Position &position);
QDataStream &operator>>	(	   QDataStream &in,		   Position &position);
bool		operator==	(const Position    &lhs, const Position &rhs);