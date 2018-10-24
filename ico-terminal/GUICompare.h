#pragma once
#include "ui_GUICompare.h"
#include <QWidget>

#include <vector>
#include <utility>
#include <map>

#include "DepthOrder.h"

QT_BEGIN_NAMESPACE
class QTimerEvent;
QT_END_NAMESPACE

using OrderBook = Entity::DepthOrder;

using PairInfo     = std::pair<uint, QString>; // id, name
using ExchangeInfo = std::pair<uint, QString>; // id, name

struct ExchangePairs
{
	ExchangeInfo          exchange;
	std::vector<PairInfo> pairs;
};

struct RowCompare
{
	ExchangeInfo exchangeBuy;
	ExchangeInfo exchangeSell;

	PairInfo pair;

	Entity::Order ask;
	Entity::Order bid;

	Entity::Order oldAsk;
	Entity::Order oldBid;
};


class GUICompare : public QWidget
{
	Q_OBJECT

public:
	GUICompare(QWidget *parent = Q_NULLPTR);
	~GUICompare() = default;

public slots:
	void onDataRecieved(const OrderBook &orderBook, const QString &index);

signals:
	void sendNotice(const QString &);
	void requestOrderBooks();

protected:
	void timerEvent(QTimerEvent *event) override;

private:
	Ui::GUICompare ui;

	std::vector<ExchangePairs> exchangePairs;
	std::vector<RowCompare>   rows;
	std::vector<uint> selectedExchange;
	std::vector<uint> selectedPairs;
	std::vector<PairInfo> currencyPair;

	std::map<QString, bool> notification;

	void initCurrnecyPair();
	void initRows();
	void initTable();

	void saveProperty();
	void loadProperty();

	void checkNotification(const QString &from, const QString &to, const QString &pair, double percent, const Entity::Order &ask, const Entity::Order &bid);

private slots:
	void start();
	void print();
	void onCurrencyPairSelected(QTableWidgetItem *item);
	void onExchangeSelected(QTableWidgetItem *item);
	void onAllSelectedExchange(int state);
	void onAllSelectedPair(int state);

	void onAddColorIndicator();
	void onRemoveColorIndicator();
};
