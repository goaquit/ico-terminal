#pragma once

#include <QWidget>
#include "ui_GuiExchangePairList.h"
#include <vector>

#include "DepthOrder.h"

class ControllerOrderBook;

using OrderBook = Entity::DepthOrder;

class GuiExchangePairList : public QWidget
{
	Q_OBJECT

public:
	GuiExchangePairList(QWidget *parent = nullptr);

	void init();
	void connectOrderBook(const ControllerOrderBook *controllerOrderBook);

signals:
	void reuqestOrderBook(const QString &index);
	void newChart(uint exchange, uint pair);
	void newOrderBook(const QString &title, const QString &index);

private:
	Ui::GuiExchangePairList ui;

	std::map<uint, std::vector<uint>> exchangePair;
	std::map<uint, QString>           pairsList;

	void initConnect();

private slots:
	void onExchangeSelected(QListWidgetItem *item);
	void onOrderBookRecieved(const OrderBook &orderBook, const QString &index);

	void createChart();
	void createOrderBook();
};
