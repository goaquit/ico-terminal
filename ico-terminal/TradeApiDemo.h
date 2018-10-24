#pragma once
#include "TradeApiWex.h"

class ControllerOrderBook;

class TradeApiDemo : public TradeApiWex
{
	Q_OBJECT
public:
	void getInfo()				override;
	void synchronize()			override;
	void openOrder(const double &price, const double &amount, Type type, const QString &pair) override;
	void orderInfo(uint id)		override;
	void cancelOrder(uint id)	override;
	void activeOrders()			override;

	void initControllerOrderBook(const ControllerOrderBook *);

signals:
	void opened(double price, double volume, Type type, const QString &pair);

};