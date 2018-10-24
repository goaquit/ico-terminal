#include "GuiPosition.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include "WrapperDb.h"
#include <memory>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QByteArray>
#include <QVariant>
#include "Options.h"
#include "GuiModifyPosition.h"
#include "GUIOrderBook.h"

using namespace TerminalCore;
using namespace Loader;

static const auto	indexOpenDate = 11, indexStatus     = 10, indexType     = 9, indexExchange = 8, 
					indexPair     = 7,	indexProfit     = 6,  indexDiff     = 5, indexCurrent  = 4, 
					indexPrice    = 3,	indexTakeProfit = 2,  indexStopLoss = 1, indexVolume   = 0;

bool is_equal(double x, double y);

static void     PrintRow(QTableWidget *table,const int row,const Position *position);
static bool     RowCheck(const QTableWidget *table, const int &row, const QString &index, const Position::Type &type);
static void     NewPrice(QTableWidget *table, const double &price, const QString &index, const PositionController &controller, const Position::Type type);
static QString  GetApiKey(const std::vector<ExchangePair> &exchangePairs, const Position *position);

GuiPosition::GuiPosition(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initAction();
	initExchangePair();

	connect(&timer, &QTimer::timeout, this, &GuiPosition::requestToPrice);
	
	connect(this, &GuiPosition::newAsk, this, &GuiPosition::onNewAsk);
	connect(this, &GuiPosition::newBid, this, &GuiPosition::onNewBid);
	connect(&controller, &PositionController::newBalance, this, &GuiPosition::onNewBalance);
	connect(&controller, &PositionController::newActiveOrders, this, &GuiPosition::onNewActiveOrders);
	connect(ui.tablePosition, &QTableWidget::doubleClicked, this, &GuiPosition::onModifyPosition);

}

void GuiPosition::initAction()
{
	// Position
	auto action = new QAction("Open position", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onAddPosition);
	ui.tablePosition->setAdd(action);

	action = new QAction("Close", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onClosePosition);
	ui.tablePosition->setClose(action);

	action = new QAction("Remove", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onRemovePosition);
	ui.tablePosition->setRemove(action);

	action = new QAction("Modify", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onModifyPosition);
	ui.tablePosition->setModify(action);

	action = new QAction("Order book", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onOrderBook);
	ui.tablePosition->setOrderBook(action);

	action = new QAction("Chart", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onChart);
	ui.tablePosition->setChart(action);

	// Order
	action = new QAction("Cancel", this);
	connect(action, &QAction::triggered, this, &GuiPosition::onCancelOrder);
	ui.tableOrders->setCancel(action);
}

void GuiPosition::initExchangePair()
{
	exchangePair.clear();

	Exchange e{ 4,"Wex" };

	const QString sql("select t2.id,t2.title,t1.key from apipair("+QString::number(e.first)+") as t1\
		left join get_available_pair() as t2 on t1.pair = t2.id;");

	std::unique_ptr<QSqlQuery> query(WrapperDb::instance()->select(sql));

	if (query->lastError().isValid())
	{
		qDebug() << "Query has error: " << query->lastError().databaseText();
		return;
	}


	std::vector<Pair> pairs;
	while (query->next())
	{
		pairs.push_back(
			{query->value(0).toUInt(),query->value(1).toString(),query->value(2).toString()}
		);
	}

	exchangePair.push_back({ e,pairs });
}

void GuiPosition::setOptions(const Options * option)
{
	controller.setOptions(option);
}

void GuiPosition::setClient(const Loader::Client &c)
{
	client = c;
	connect(&client, &Client::blockReady, this, &GuiPosition::onDataRecieved);
	connect(&client, &Client::isReady, [this]()
	{
		timer.stop();
		timer.setInterval(1000);
		timer.start();
	});

	client.start();
}

void GuiPosition::setProfileName(const QString &name)
{
	controller.setProfile(name);

	// getting positions and add them to table
	for (const auto position : controller.getPositions())
	{
		appendToTable(position);
	}
}

void GuiPosition::onClosePosition()
{
	if (!ui.tablePosition->rowCount()) return;

	const auto row = ui.tablePosition->currentRow();
	const auto currentPrice = ui.tablePosition->item(row, indexCurrent)->data(Qt::DisplayRole).toDouble();

	if (currentPrice <= 0.0) return;

	const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

	auto position = controller.getPosition(checkedHash);

	if (position->getVolCur() <= 0.0)
	{
		controller.remove(position);

		emit position->sendMessage(
			"Removing position " + QString((position->getType() == Position::Long) ? "Long " : "Short ")
			+ " price: " + QString::number(position->getPrice()) + " volume: " + QString::number(position->getVolCur()) + " " + position->getPair()
		);
	}
	else
	{
		position->close(currentPrice);
	}
}

void GuiPosition::onRemovePosition()
{
	const auto row         = ui.tablePosition->currentRow();
	const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

	if (!checkedHash.size()) return;

	auto position = controller.getPosition(checkedHash);

	if (!position) return;

	emit position->sendMessage(
		"Removing position " + QString((position->getType() == Position::Long) ? "Long " : "Short ")
		+ " price: " + QString::number(position->getPrice()) + " volume: " + QString::number(position->getVolCur()) + " " + position->getPair()
	);
	controller.remove(position );
}

void GuiPosition::requestToPrice()
{
	for (const auto &exchange : exchangePair)
	{
		for (const auto &pair : exchange.second)
		{
			const QString index(
				exchange.first.second + "_" + std::get<1>(pair) + ":1#"
			);

			client.send(index.toLower());
		}
	}
}

void GuiPosition::onModifyPosition()
{
	if (!ui.tablePosition->rowCount()) return;

	const auto row = ui.tablePosition->currentRow();
	const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

	if (!checkedHash.size()) return;

	auto position = controller.getPosition(checkedHash);

	if (!position) return;

	auto w(new GuiModifyPosition);
	w->setPosition(position);
	w->show();

	auto connectNewAsk = connect(this, &GuiPosition::newAsk, w, &GuiModifyPosition::onNewAsk);
	auto connectNewBid = connect(this, &GuiPosition::newBid, w, &GuiModifyPosition::onNewBid);

	connect(w, &GuiModifyPosition::destroyed, [this, connectNewAsk, connectNewBid, w]()
	{
		this->disconnect(connectNewAsk); this->disconnect(connectNewBid); w->close();
	});

	connect(w, &GuiModifyPosition::modifyed, [this,position]()
	{
		changedPosition(position);
	});
}

void GuiPosition::onOrderBook()
{
	if (!ui.tablePosition->rowCount()) return;

	const auto row = ui.tablePosition->currentRow();
	const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

	if (!checkedHash.size()) return;

	auto position = controller.getPosition(checkedHash);

	if (!position) return;

	QString windowTitle = "OrderBook " + position->getExchange() + "::" + position->getPair();


	const QString index = (position->getExchange().replace(".", "") + "_" + position->getPair()).toLower();

	auto window = new Terminal::GUIOrderBook(this);

	window->setIndex(index);

	emit sendToMdi(window);
	window->setWindowTitle(windowTitle);
	window->show();
}

void GuiPosition::onChart()
{
	if (!ui.tablePosition->rowCount()) return;

	const auto row = ui.tablePosition->currentRow();
	const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

	if (!checkedHash.size()) return;

	auto position = controller.getPosition(checkedHash);

	if (!position) return;

	uint e = 0;
	uint p = 0;
	for (const auto &exchange : exchangePair)
	{
		if (exchange.first.second.compare(position->getExchange(), Qt::CaseInsensitive) != 0) continue;
		for (const auto &pair : exchange.second)
		{
			const auto key = std::get<1>(pair);
			if (!key.compare(position->getPair(), Qt::CaseInsensitive))
			{
				e = exchange.first.first; p = std::get<0>(pair);

				break;
			}
		}
	}

	if (!e || !p) return;

	emit requestToCreateChart(e, p);
}

void GuiPosition::onCancelOrder()
{
	if (!ui.tableOrders->rowCount()) return;

	const auto row = ui.tableOrders->currentRow();
	const auto id = ui.tableOrders->item(row, 0)->data(Qt::UserRole + QVariant::UInt).toUInt();

	if (!id) return;

	controller.cancelOrder(id);

	ui.tableOrders->removeRow(row);
}

void GuiPosition::onDataRecieved(QByteArray &data)
{
	QDataStream reader(&data, QIODevice::ReadOnly);


	quint32 sizeBlock = 0;
	quint8  marker = 0;
	quint16 countBids = 0;
	quint16 countAsks = 0;


	reader >> sizeBlock >> marker >> countBids >> countAsks;

	reader.skipRawData(2 * sizeof(double));

	// read index
	quint16 sizeIndex = 0;


	reader >> sizeIndex;

	std::unique_ptr<char> chrIndex(new char[sizeIndex + 1]{ 0 });
	reader.readRawData(chrIndex.get(), sizeIndex);

	QString index(chrIndex.get());

	if (!countBids || !countAsks) return;

	// Bids
	for (auto i = 0; i < countBids; ++i)
	{
		double price = 0.0;
		double amount = 0.0;

		reader >> price >> amount;

		emit newBid(price, index);
	}

	// Asks
	for (auto i = 0; i < countAsks; ++i)
	{
		double price = 0.0;
		double amount = 0.0;

		reader >> price >> amount;

		emit newAsk(price, index);
	}

}

void GuiPosition::onNewAsk(double price, const QString &index)
{
	if (price <= 0.0 || !index.size()) return;

	NewPrice(ui.tablePosition, price, index, controller, Position::Short);
}

void GuiPosition::onNewBid(double price, const QString &index)
{
	if (price <= 0.0 || !index.size()) return;

	NewPrice(ui.tablePosition, price, index, controller, Position::Long);
}

void GuiPosition::onOpenPosition(Position *position, double price)
{
	position->setApiPair(GetApiKey(exchangePair, position));
	controller.append(position); 

	connect(position, &Position::error, [this, position](const QString &msg)
	{
		qDebug() << msg;
		controller.remove(position);
		removeFromTable(position);
	});

	appendToTable(position);
	
	position->open(price);

	emit sendMessage(
		"Opening position " + QString((position->getType() == Position::Long) ? "Long " : "Short ")
		+ " price: " + QString::number(position->getPrice()) + " volume: " + QString::number(position->getVolume()) + " "  + position->getPair()
	);
}

void GuiPosition::onNewBalance(const QVector<QPair<QString, double>> &balance)
{
	ui.tableBalance->clearContents();
	ui.tableBalance->setRowCount(balance.size());
	int row = 0;
	for (const auto &pair : balance)
	{
		ui.tableBalance->setItem(row, 0, new QTableWidgetItem(pair.first.toUpper()));
		ui.tableBalance->setItem(row, 1, new QTableWidgetItem(QString::number(pair.second),QVariant::Double));
		row++;
	}
}

void GuiPosition::onNewActiveOrders(const QVector<TradeOrderInfo> &activeOrders)
{
	ui.tableOrders->clearContents();
	ui.tableOrders->setRowCount(activeOrders.size());

	int row = 0;
	for (const auto &order : activeOrders)
	{
		auto column = 0;
		auto item = new QTableWidgetItem(QString::number(order.getRate()), QVariant::Double);
		item->setData(Qt::UserRole + QVariant::UInt, order.getId());
		ui.tableOrders->setItem(row, column++,	item);
		ui.tableOrders->setItem(row, column++, new QTableWidgetItem( QString::number(order.getAmount()), QVariant::Double)	);
		ui.tableOrders->setItem(row, column++, new QTableWidgetItem( QDateTime::fromTime_t(order.getCreateTimestamp()).toString(Qt::ISODate))	);
		ui.tableOrders->setItem(row, column++, new QTableWidgetItem( order.getPair()	));

		row++;
	}
}

void GuiPosition::appendToTable(const Position *position)
{
	const auto row = ui.tablePosition->rowCount();
	connect(position, &Position::changed,  this, &GuiPosition::changedPosition);
	const auto hash = position->getHash();
	connect(position, &Position::destroyed, [this, hash]()
	{	
		removeFromTable(hash);
	});

	connect(position, &Position::error, this, &GuiPosition::sendMessage);
	connect(position, &Position::sendMessage, this, &GuiPosition::sendMessage);

	ui.tablePosition->setRowCount(row + 1);

	PrintRow(ui.tablePosition, row, position);
}

void GuiPosition::removeFromTable(const Position *position)
{
	for (auto row = 0; row < ui.tablePosition->rowCount(); ++row)
	{
		const auto hash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();
		if (hash == position->getHash())
		{
			ui.tablePosition->removeRow(row);
		}
	}
}

void GuiPosition::changedPosition(const Position *position)
{
	const auto rows = ui.tablePosition->rowCount();
	if (!rows)
	{
		appendToTable(position);
		return;
	}

	// find row
	auto row = 0;
	for (; row < rows; ++row)
	{
		const auto indexPrice = 3;
		const auto checkedHash = ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray();

		if (checkedHash == position->getHash()) break;
	}

	if (row == rows) return;
		
	// status
	QString status("-");
	switch (position->getStatus())
	{
	case Position::Open:	status = "Open";	break;
	case Position::Opening: status = "Opening"; break;
	case Position::Close:	status = "Close";	break;
	case Position::Closing: status = "Closing"; break;
	}

	ui.tablePosition->item(row, indexStatus)->setData(Qt::DisplayRole, status);
	
	// volume
	const auto volume = !is_equal(position->getVolCur(), position->getVolume()) ?
		QString::number(position->getVolCur()) + "/" + QString::number(position->getVolume()) : QString::number(position->getVolume());
	ui.tablePosition->item(row, indexVolume)->setData(Qt::DisplayRole, volume);

	// stopLoss
	ui.tablePosition->item(row, indexStopLoss)->setData(Qt::DisplayRole, position->getStopLoss());

	// takeProfit
	ui.tablePosition->item(row, indexTakeProfit)->setData(Qt::DisplayRole, position->getTakeProfite());
}

void GuiPosition::removeFromTable(const QByteArray &hash)
{
	const auto rowCount = ui.tablePosition->rowCount();

	for (auto row = 0; row < rowCount; ++row)
	{
		if (hash != ui.tablePosition->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray()) continue;

		ui.tablePosition->removeRow(row); break;
	}
}

void GuiPosition::onAddPosition()
{
	if (!controller.isReady()) return;

	auto w(new GuiOpenPosition);
	w->setExchangePairs(exchangePair);
	w->show();

	connect(w, &GuiOpenPosition::openPosition, this, &GuiPosition::onOpenPosition);
	auto connectNewAsk = connect(this, &GuiPosition::newAsk, w, &GuiOpenPosition::onNewAsk);
	auto connectNewBid = connect(this, &GuiPosition::newBid, w, &GuiOpenPosition::onNewBid);
}

static void PrintRow(QTableWidget *table,const int row, const Position *position)
{
	auto column = 0;

	// volume
	auto item = new QTableWidgetItem;

	const auto volume = !is_equal(position->getVolCur(), position->getVolume()) ?
		QString::number(position->getVolCur()) + "/" + QString::number(position->getVolume()) : QString::number(position->getVolume());

	// volume
	item->setData(Qt::DisplayRole, volume);
	table->setItem(row, column, item);
	column++;

	// stop loss
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getStopLoss());
	table->setItem(row, column, item);
	column++;

	// take profit
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getTakeProfite());
	table->setItem(row, column, item);
	column++;

	// price
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getPrice());
	item->setData(Qt::UserRole + QVariant::ByteArray, position->getHash());
	table->setItem(row, column, item);
	column++;

	// current
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getStopLoss());
	table->setItem(row, column, item);
	column++;

	// diff
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, 0);
	table->setItem(row, column, item);
	column++;

	// profit
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, 0);
	table->setItem(row, column, item);
	column++;

	// pair
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getPair().toUpper());
	table->setItem(row, column, item);
	column++;

	// exchange
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, position->getExchange()); //todo: make exchange 
	table->setItem(row, column, item);
	column++;

	// type
	const auto type = (position->getType() == Position::Long) ? "Long" : "Short";
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, type);
	table->setItem(row, column, item);
	column++;

	// status
	QString status("-");
	switch (position->getStatus())
	{
	case Position::Open:	status = "Open";	break;
	case Position::Opening: status = "Opening"; break;
	case Position::Close:	status = "Close";	break;
	case Position::Closing: status = "Closing"; break;
	}
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, status);
	table->setItem(row, column, item);
	column++;

	// date
	const auto date = QDateTime::fromTime_t(position->getDateOpen());
	item = new QTableWidgetItem;
	item->setData(Qt::DisplayRole, date.toString(Qt::ISODate));
	table->setItem(row, column, item);
	column++;

}

bool RowCheck(const QTableWidget * table, const int & row, const QString & index, const Position::Type & type)
{
	auto result = false;

	QString typeStr = "";

	switch (type)
	{
		case Position::Long:  typeStr = "Long";  break;
		case Position::Short: typeStr = "Short"; break;
	}

	if (!typeStr.size()) return result;

	result = table->item(row, indexType)->text().compare(typeStr, Qt::CaseInsensitive) == 0;

	const auto exchange = table->item(row, indexExchange)->text();
	const auto pair     = table->item(row, indexPair)->text();

	result = result && (index.compare(QString(exchange + "_" + pair), Qt::CaseInsensitive) == 0);

	return result;
}

void NewPrice(QTableWidget * table, const double & price, const QString & index, const PositionController & controller, const Position::Type type)
{
	const auto rowCount =table->rowCount();

	for (auto row = 0; row < rowCount; ++row)
	{
		if (!RowCheck(table, row, index, type)) continue;

		const auto position = controller.getPosition(table->item(row, indexPrice)->data(Qt::UserRole + QVariant::ByteArray).toByteArray());

		if (!position) continue;

		const auto diff = (position->getType() == Position::Long) ? price - position->getPrice() : position->getPrice() - price;

		// print new value 
		table->item(row, indexCurrent)->setData(Qt::DisplayRole, price);
		table->item(row, indexDiff)->setData(Qt::DisplayRole, diff);
		table->item(row, indexProfit)->setData(Qt::DisplayRole, diff * position->getVolCur());
		// status
		QString status("-");
		switch (position->getStatus())
		{
		case Position::New:     status = "New";     break;
		case Position::Open:	status = "Open";	break;
		case Position::Opening: status = "Opening"; break;
		case Position::Close:	status = "Close";	break;
		case Position::Closing: status = "Closing"; break;
		}

		table->item(row, indexStatus)->setData(Qt::DisplayRole, status);

		// check stop loss and take profit
		emit position->check(price);
	}
}

QString GetApiKey(const std::vector<ExchangePair>& exchangePairs, const Position *position)
{
	QString key = "";

	for (const auto &exchange : exchangePairs)
	{
		if (exchange.first.second.compare(position->getExchange())) continue;

		for (const auto &pair : exchange.second)
		{
			if (std::get<1>(pair).compare(position->getPair())) continue;

			key = std::get<2>(pair);

			break;
		}

		break;
	}

	return key;
}