#include "GUIOrderBook.h"

#include <QSqlQuery>

#include <QString>
#include <QDataStream>
#include "ControllerOrderBook.h"

const int defaultDepth = 15;

static inline int ProcessAlpha(double percent)
{
	return (percent >= 1) ? 100 : (percent >= 0.1) ? 75 : (percent >= 0.05) ? 50 : 20;
}


namespace Terminal
{


GUIOrderBook::GUIOrderBook(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.spinBoxDepth->setValue(defaultDepth);

	QObject::connect(ui.spinBoxDepth, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GUIOrderBook::changeDepth);
}

void GUIOrderBook::setIndex(const QString &value)
{
	index = value;
}

void GUIOrderBook::connectControllerOrderBook(const ControllerOrderBook * controllerOrderBook)
{
	connect(controllerOrderBook, &ControllerOrderBook::recievedOrderBook, this, &GUIOrderBook::onOrderBookRecieved);
}

void GUIOrderBook::resizeEvent(QResizeEvent * event)
{
	auto widthCol = event->size().width() / 3;

	ui.tableOrders->setColumnWidth(0, widthCol);
	ui.tableOrders->setColumnWidth(1, widthCol);
	ui.tableOrders->setColumnWidth(2, widthCol);

	QMainWindow::resizeEvent(event);
}
	
void GUIOrderBook::print() const
{
	ui.tableOrders->clearContents();

	const auto count = ui.spinBoxDepth->value();
	ui.tableOrders->setRowCount(2 * count);
		
	int counter = 0;
	int row = count - 1;

	for (const auto &order : orderBook.asks())
	{
		if (counter >= count) break;

		auto itemPrice = new QTableWidgetItem(QString::number(order.getPrice(), 'f', 4));
		auto itemAmount = new QTableWidgetItem(QString::number(order.getAmount(), 'f', 6));

		// colors set
		auto color = QColor(Qt::red);

		const auto percentAmount = (1 - (orderBook.quantityAsks() - order.getAmount()) / orderBook.quantityAsks()) * 100;
		color.setAlpha(ProcessAlpha(percentAmount));

		itemAmount->setBackground(QBrush(color));
		itemPrice->setBackground(QBrush(color));

		auto font = itemAmount->font();
		font.setBold(percentAmount >= 1);

		itemAmount->setFont(font);
		itemPrice->setFont(font);

		ui.tableOrders->setItem(row, 1, itemPrice);
		ui.tableOrders->setItem(row, 2, itemAmount);

		--row;
		counter++;
	}

	counter = 0;
	row = count;
	for (const auto &order : orderBook.bids())
	{
		if (counter >= count) break;

		auto itemPrice = new QTableWidgetItem(QString::number(order.getPrice(), 'f', 4));
		auto itemAmount = new QTableWidgetItem(QString::number(order.getAmount(), 'f', 6));

		// colors set
		auto color = QColor(Qt::green);
		const auto percentAmount = (1 - (orderBook.quantityBids() - order.getAmount()) / orderBook.quantityBids()) * 100;
		color.setAlpha(ProcessAlpha(percentAmount));

		itemAmount->setBackground(QBrush(color));
		itemPrice->setBackground(QBrush(color));

		auto font = itemAmount->font();
		font.setBold(percentAmount >= 1);

		itemAmount->setFont(font);
		itemPrice->setFont(font);

		ui.tableOrders->setItem(row, 1, itemPrice);
		ui.tableOrders->setItem(row, 0, itemAmount);

		counter++;
		row++;
	}



	ui.labelBuyPrice->setText(
		QString::number(orderBook.bids().begin()->getPrice(), 'f', 4)
	);

	ui.labelSellPrice->setText(
		QString::number(orderBook.asks().begin()->getPrice(), 'f', 4)
	);
}

void GUIOrderBook::changeDepth(int)
{
	print();
}

void GUIOrderBook::onOrderBookRecieved(const OrderBook & o, const QString & i)
{
	if (i.compare(index, Qt::CaseInsensitive)) return;

	orderBook = o;

	print();
}



}