#pragma once

#include <QWidget>
#include "ui_GuiOpenPosition.h"


namespace Ui
{
	class WidgetOrderLimit;
	class WidgetOrderMarket;
}

class Position;

using Exchange = std::pair<uint, QString>;
// id, name, apiKey
using Pair = std::tuple<uint, QString, QString>;

using ExchangePair = std::pair<Exchange, std::vector<Pair>>;

class GuiOpenPosition : public QWidget
{
	Q_OBJECT

public:
	GuiOpenPosition(QWidget *parent = Q_NULLPTR);

	void setExchangePairs(const std::vector<ExchangePair> &v);
	
public slots:
	void onNewAsk(double, const QString &);
	void onNewBid(double, const QString &);
	void onError(const QString &msg);

signals:
	void openPosition(Position *position, double price = 0.0);

private:
	Ui::GuiOpenPosition ui;

	QWidget *orderType = nullptr;

private slots:
	void onOrderLimit();
	void onOrderMarket();
};
