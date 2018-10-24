#pragma once

#include <QObject>
#include <map>
#include <QVector>
#include <map>
#include "DepthOrder.h"


using OrderBook = Entity::DepthOrder;
class TradeApiDemo;

// QString exchange, QString currency, double volume
using TupleBalance = std::tuple<QString, QString, double>;

namespace Entity
{

using uptrTradeApi = std::unique_ptr<TradeApiDemo>;
using mapBalance   = std::map<QString, double>;


class Balance : public QObject
{
	Q_OBJECT

public:
	Balance(QObject *parent = nullptr);
	~Balance() = default;

	void synchronize();

	inline mapBalance  getBalance() const {	return balance;	}

signals:
	void changed();
	void sold(double volume, double price);
	void bought(double volume, double price);
	void error(const QString &msg);

public slots:
	void buy(double volume, double price, const QString &pair);
	void sell(double volume, double price, const QString &pair);

private:
	// QString currency, double balance
	mapBalance balance;

	uptrTradeApi tradeApi;

private slots:
	void onGetBalance(const QVector<QPair<QString, double>> &balanceList);
	
};

}