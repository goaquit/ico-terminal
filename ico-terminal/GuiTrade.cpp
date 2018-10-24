#include "GuiTrade.h"
#include "ControllerOrderBook.h"
#include "Balance.h"
#include <QListWidget>

static void AddMsgToLog(QListWidget *list, const QString &msg)
{
	list->addItem(msg);
	list->scrollToBottom();
}

GuiTrade::GuiTrade(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.buy, &QPushButton::clicked, this, &GuiTrade::onBuyButton);
	connect(ui.sell, &QPushButton::clicked, this, &GuiTrade::onSellButton);
	connect(ui.exchange, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &GuiTrade::onChangeExchange);
	connect(ui.pair, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &GuiTrade::onChangePair);
}

void GuiTrade::connectOrderBook(const ControllerOrderBook * controller)
{
	connect(controller, &ControllerOrderBook::recievedOrderBook, this, &GuiTrade::onOrderBookRecieved);
	connect(this, &GuiTrade::requestOrderBook, controller, &ControllerOrderBook::requestOrderBook);
}

void GuiTrade::connectBalance(const Entity::Balance * balance)
{
	connect(this, &GuiTrade::buy,  balance, &Entity::Balance::buy);
	connect(this, &GuiTrade::sell, balance, &Entity::Balance::sell);

	connect(balance, &Entity::Balance::bought, this, &GuiTrade::onBuy);
	connect(balance, &Entity::Balance::sold,   this, &GuiTrade::onSell);
	connect(balance, &Entity::Balance::error,  this, &GuiTrade::onError);
}

void GuiTrade::onBuyButton()
{
	const auto volume = ui.volume->value();
	const auto price = ui.ask->value();
	if (volume == 0.0 || price == 0.0)
	{
		AddMsgToLog(ui.log, QString("Volume is null"));
		return;
	};
	//todo: need index trade
	emit buy(volume, price,ui.pair->currentText());
}

void GuiTrade::onSellButton()
{
	ui.buy->setEnabled(false);
	ui.sell->setEnabled(false);

	const auto volume = ui.volume->value();
	const auto price = ui.bid->value();
	if (volume == 0.0 || price == 0.0)
	{
		AddMsgToLog(ui.log,QString("Volume is null"));

		ui.buy->setEnabled(true);
		ui.sell->setEnabled(true);

		return;
	};
	//todo: need index trade
	emit sell(volume, price, ui.pair->currentText());
}

void GuiTrade::onChangePair(const QString & text)
{
	const auto index = ui.exchange->currentText() + "_" + text;
	emit requestOrderBook(index);
}

void GuiTrade::onChangeExchange(const QString & text)
{
	const auto index = text + "_" + ui.pair->currentText();
	emit requestOrderBook(index);
}

void GuiTrade::onBuy(double volume, double price)
{
	ui.buy->setEnabled(true);
	ui.sell->setEnabled(true);

	AddMsgToLog(ui.log,
		QString("Buy - Price: %1 - Volume: %2 - Cost: %3")
		.arg(price).arg(volume, 0, 'f', 4).arg(price * volume));
}

void GuiTrade::onSell(double volume, double price)
{
	ui.buy->setEnabled(true);
	ui.sell->setEnabled(true);

	AddMsgToLog(ui.log,
		QString("Sell - Price: %1 - Volume: %2 - Cost: %3")
		.arg(price).arg(volume, 0, 'f', 4).arg(price * volume));
}

void GuiTrade::onError(const QString & msg)
{
	ui.buy->setEnabled(true);
	ui.sell->setEnabled(true);

	AddMsgToLog(ui.log, msg);
}

void GuiTrade::onOrderBookRecieved(const OrderBook & orderBook, const QString & index)
{
	const auto currIndex = ui.exchange->currentText() + "_" + ui.pair->currentText();

	if (currIndex.compare(index, Qt::CaseInsensitive) ||
		(!orderBook.asks().size() || !orderBook.bids().size())
		) return;

	ui.ask->setValue(orderBook.asks().begin()->getPrice());
	ui.bid->setValue(orderBook.bids().begin()->getPrice());
}
