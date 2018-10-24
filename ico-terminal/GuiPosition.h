#pragma once

#include <QWidget>
#include "ui_GuiPosition.h"
#include "PositionController.h"
#include "Client.h"
#include <vector>
#include <QTimer>
#include "GuiOpenPosition.h"

class QAction;
class Options;

class GuiPosition : public QWidget
{
	Q_OBJECT

public:
	GuiPosition(QWidget *parent = Q_NULLPTR);
	~GuiPosition() = default;

	void setClient(const Loader::Client &);
	void setProfileName(const QString &);
	void setOptions(const Options *option);

private:
	Ui::GuiPosition ui;

	PositionController controller;

	// action
	void initAction();

	// pairs
	Loader::Client client;
	std::vector<ExchangePair> exchangePair;
	QTimer timer;
	void initExchangePair();
signals:
	void sendMessage(const QString &msg);
	void requestToCreateChart(uint exchange, uint pair);
	void sendToMdi(QWidget *);

private slots:
	// action
	void onAddPosition();
	void onClosePosition();
	void onRemovePosition();
	void onModifyPosition();
	void onOrderBook();
	void onChart();
	void onCancelOrder();

	//from trade
	void onNewAsk(double, const QString &);
	void onNewBid(double, const QString &);
	void onNewBalance(const QVector<QPair<QString, double>> &);
	void onNewActiveOrders(const QVector<TradeOrderInfo> &);

	void requestToPrice();
	void onDataRecieved(QByteArray &);	
	void onOpenPosition(Position *, double price);
	
	// operation with table
	void appendToTable(const Position *);
	void removeFromTable(const Position *);
	void changedPosition(const Position *);	
	void removeFromTable(const QByteArray &hash); // removing by hash position

	
signals:
	void newBid(double, const QString &);
	void newAsk(double, const QString &);
	void takeProfit();
	void stopLoss();
};
