#pragma once
#include <QObject>


class TradeOrderInfo : public QObject
{
	Q_OBJECT
public:

	enum Type : uint
	{
		None,
		Bye,
		Sell
	};

	enum Status : uint
	{
		Active,
		Executed,
		Canceled,
		Partial
	};

	TradeOrderInfo(QObject *parent = nullptr) : QObject(parent) {};
	TradeOrderInfo(uint id, QObject *parent = nullptr);
	TradeOrderInfo(uint id, const QString &pair, double startAmount, double amount, double rate, time_t createTimestamp, Type type, Status status, QObject *parent = nullptr);
	TradeOrderInfo(const TradeOrderInfo &);
	TradeOrderInfo & operator=(const TradeOrderInfo &copy);


	///////////////////////////////////////
	// setters
	void setId(const uint &);
	void setPair(const QString &);
	void setStartAmount(const double &);
	void setAmount(const double &);
	void setRate(const double &);
	void setCreateTimestamp(const time_t &);
	void setType(const Type &);
	void setStatus(const Status &);

	///////////////////////////////////////
	// getters
	uint	getId()					const {	return id;				}
	QString getPair()				const {	return pair;			}
	double	getStartAmount()		const {	return startAmount;		}
	double	getAmount()				const {	return amount;			}
	double	getRate()				const {	return rate;			}
	time_t	getCreateTimestamp()	const {	return createTimestamp;	}
	Type	getType()				const {	return type;			}
	Status	getStatus()				const {	return status;			}


private:
	uint	id				= 0;
	QString pair			= "";
	double	startAmount		= 0.0;
	double	amount			= 0.0;
	double	rate			= 0.0;
	time_t	createTimestamp = 0;
	Type	type			= None;
	Status  status			= Active;
};

QDataStream &operator<<(QDataStream &out, const TradeOrderInfo &order);
QDataStream &operator>>(QDataStream &in,		TradeOrderInfo &order);

bool operator==(const TradeOrderInfo& lhs, const TradeOrderInfo& rhs);
bool operator!=(const TradeOrderInfo& lhs, const TradeOrderInfo& rhs);

/*

pair: Пара на которой был создан ордер.

type: Тип ордера, buy/sell.

start_amount: Начальная сумма которая была при создании ордера.

amount: Сколько осталось купить/продать.

rate: Цена покупки/продажи.

timestamp_created: Время когда был создан ордер.

status: 0 - активен, 1 - исполненный ордер, 2 - отмененный, 3 - отмененный, но был частично исполнен.

*/