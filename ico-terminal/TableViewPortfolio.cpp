#include "TableViewPortfolio.h"
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QPoint>
#include "GuiTrade.h"
#include "ControllerOrderBook.h"
#include "Balance.h"

TableViewPortfolio::TableViewPortfolio(QWidget *parent) :QTableView(parent)
{
	initAction();
	windowTrade = new GuiTrade;
	windowTrade->setWindowTitle("Trade");
	connect(this, &QObject::destroyed, [this]()
	{
		windowTrade->close();
		windowTrade->deleteLater();
	});
}

void TableViewPortfolio::connectControllerOrderBook(const ControllerOrderBook *v)
{
	windowTrade->connectOrderBook(v);
}

void TableViewPortfolio::connectBalance(const Entity::Balance *v)
{
	windowTrade->connectBalance(v);
}

void TableViewPortfolio::contextMenuEvent(QContextMenuEvent * e)
{
	QMenu menu(this);
	menu.addAction(actionTrade);
	menu.exec(e->globalPos());
}

void TableViewPortfolio::onTrade()
{
	windowTrade->show();
}

void TableViewPortfolio::initAction()
{
	actionTrade  = new QAction("Trade",this);

	connect(actionTrade,  &QAction::triggered, this, &TableViewPortfolio::onTrade);
}
