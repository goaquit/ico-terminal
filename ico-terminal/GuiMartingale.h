#pragma once

#include <QWidget>
#include "ui_GuiMartingale.h"
#include "Martingale.h"

namespace Entity
{
class Balance;
}

class ControllerOrderBook;

class GuiMartingale : public QWidget
{
	Q_OBJECT

public:
	GuiMartingale(QWidget *parent = Q_NULLPTR);
	~GuiMartingale() = default;

	void initConnect(const Entity::Balance *);

signals:
	void reuqestAsk(const QString &index);

public slots:
	void onChangedAsk(double price, const QString &index);
	void onChangedBid(double price, const QString &index);
	void onError(const QString &msg);
	void onBuy(double volulme, double price);
	void onSell(double volulme, double price);
	void onStopLossChangedValue(double);
	void onStopLossChangedPercent(double);
	void onProfit(double);

protected:
	void timerEvent(QTimerEvent *event) override;

private:
	Ui::GuiMartingale ui;

	Martingale martingale;

	int timerId = 0;

private slots:
	void start();
	void stop();
	void sell();
	void rest();
	void changedPair(const QString &pair);
	void changedExchange(const QString &exchange);
	
};
