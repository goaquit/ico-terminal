#include "GuiExchangePairList.h"
#include "ControllerOrderBook.h"

#include "WrapperDb.h"
#include "SqlRequest.h"

using namespace TerminalCore;

GuiExchangePairList::GuiExchangePairList(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

void GuiExchangePairList::init()
{
	initConnect();

	auto query = WrapperDb::instance()->select(SqlRequest::exchangeAvailableList);

	std::vector<uint> exchangeList;
	while (query->next())
	{
		const auto id		= query->value(0).toUInt();
		const auto exchange = query->value(1).toString();

		auto item = new QListWidgetItem(exchange.trimmed(), ui.exchange);
		item->setData(Qt::UserRole, id);

		exchangeList.push_back(id);
	}

	delete query;

	// load pairs map
	query = WrapperDb::instance()->select(SqlRequest::pairAvailableList);

	while (query->next())
	{
		const auto id   = query->value(0).toUInt();
		const auto pair = query->value(1).toString();

		pairsList[id] = pair.toUpper();
	}
	delete query;

	// connect exchange to pair list
	for (const auto &exchange : exchangeList)
	{
		query = WrapperDb::instance()->select(SqlRequest::getPairExchange.arg(exchange));
		auto vector = exchangePair[exchange];

		while (query->next())
		{
			const auto pair = query->value(0).toUInt();

			vector.push_back(pair);
		}

		exchangePair[exchange] = vector;
	}
}

void GuiExchangePairList::connectOrderBook(const ControllerOrderBook * controllerOrderBook)
{
	ui.exchange->clear();
	ui.pair->clearContents();

	connect(controllerOrderBook, &ControllerOrderBook::recievedOrderBook, this, &GuiExchangePairList::onOrderBookRecieved);
	connect(this, &GuiExchangePairList::reuqestOrderBook, controllerOrderBook, &ControllerOrderBook::requestOrderBook);
}

void GuiExchangePairList::initConnect()
{
	connect(ui.exchange, &QListWidget::itemClicked, this, &GuiExchangePairList::onExchangeSelected);
	connect(ui.chart, &QPushButton::clicked, this, &GuiExchangePairList::createChart);
	connect(ui.orderBook, &QPushButton::clicked, this, &GuiExchangePairList::createOrderBook);
}

void GuiExchangePairList::onOrderBookRecieved(const OrderBook & ob, const QString & index)
{
	if (!ui.exchange->selectedItems().size()) return;

	const auto sell = *ob.asks().begin();
	const auto buy  = *ob.bids().begin();

	// find current exchange
	const auto exchange = (*ui.exchange->selectedItems().cbegin())->text().replace(".", "").toLower();

	// check pair and find row
	for (auto row = 0; row < ui.pair->rowCount(); ++row)
	{
		const auto item = ui.pair->item(row, 0);

		const auto pair = item->text().replace("/", "").toLower();

		const auto currentIndex = exchange + "_" + pair;

		if (!index.compare(currentIndex,Qt::CaseInsensitive))
		{
			auto itemBuy  = ui.pair->item(row, 1);
			auto itemSell = ui.pair->item(row, 2);

			itemBuy->setText(QString::number(buy.getPrice()));
			itemSell->setText(QString::number(sell.getPrice()));

			break;
		}
	}
}

void GuiExchangePairList::createChart()
{
	const auto itemPair = ui.pair->selectedItems().at(0);

	if (!itemPair) return;

	const auto listExchange = ui.exchange->selectedItems();

	uint exchange = 0;

	for (const auto &item : listExchange)
	{
		exchange = item->data(Qt::UserRole).toUInt();
	}

	uint pair = itemPair->data(Qt::UserRole + QVariant::UInt).toUInt();

	if (!exchange || !pair) return;

	emit newChart(exchange, pair);
}

void GuiExchangePairList::createOrderBook()
{
	const auto itemPair = ui.pair->selectedItems().at(0);

	if (!itemPair) return;

	const auto listExchange = ui.exchange->selectedItems();

	QString exchangeName;
	QString windowTitle;

	for (const auto &item : listExchange)
	{
		windowTitle += "OrderBook " + item->text() + "::";
		exchangeName = item->text().toLower() + "_";
	}

	QString index;

	exchangeName.replace(".", "");
	const auto pairStr = itemPair->text();
	index = exchangeName.toLower() + pairStr.toLower();
	windowTitle += pairStr.toUpper();


	emit newOrderBook(windowTitle, index);
}

void GuiExchangePairList::onExchangeSelected(QListWidgetItem * item)
{
	ui.pair->clearContents();

	const auto exchange      = item->data(Qt::UserRole).toUInt();
	const auto exchangeTitle = item->data(Qt::DisplayRole).toString().replace(".", "").toLower();

	int rowMax = static_cast<int>(exchangePair[exchange].size());
	ui.pair->setRowCount(rowMax);
	
	uint row = 0;
	for (const auto &pair : exchangePair[exchange])
	{
		const auto pairTitle = pairsList[pair];

		auto pairItem = new QTableWidgetItem(pairTitle);
		pairItem->setData(Qt::UserRole + QVariant::UInt, pair);

		auto itemBuy  = new QTableWidgetItem();
		auto itemSell = new QTableWidgetItem();

		ui.pair->setItem(row, 0, pairItem);
		ui.pair->setItem(row, 1, itemBuy);
		ui.pair->setItem(row, 2, itemSell);

		const auto index = (exchangeTitle + "_" + pairTitle).toLower();

		emit reuqestOrderBook(index);

		row++;
	}
}
