#pragma once

#include <QWidget>
#include "ui_GuiTrade.h"
#include "DepthOrder.h"

using OrderBook = Entity::DepthOrder;

class ControllerOrderBook;
namespace Entity
{
class Balance;
}


class GuiTrade : public QWidget
{
	Q_OBJECT
public:
	GuiTrade(QWidget *parent = nullptr);

	void connectOrderBook(const ControllerOrderBook *controller);
	void connectBalance(const Entity::Balance *balance);

signals:
	void buy(double volume, double price, const QString &pair);
	void sell(double volume, double price, const QString &pair);
	void requestOrderBook(const QString &index);

private:
	Ui::GuiTrade ui;

private slots:
	void onOrderBookRecieved(const OrderBook &orderBook, const QString &index);
	void onBuyButton();
	void onSellButton();
	void onChangePair(const QString &text);
	void onChangeExchange(const QString &text);
	void onBuy(double volume, double price);
	void onSell(double volume, double price);
	void onError(const QString &msg);
};
