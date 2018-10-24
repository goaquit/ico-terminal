#include "TablePortfolio.h"
#include "Balance.h"
#include "PortfolioRow.h"
#include "ControllerOrderBook.h"

TablePortfolio::TablePortfolio(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.tableView->setModel(&model);
	ui.tableView->horizontalHeader()->setStretchLastSection(true);

	initConnect();

	onSelectCurrency(ui.currency->currentText());
}

void TablePortfolio::onSelectCurrency(const QString & text)
{
	model.changeCurrencyTotal(text);
}

void TablePortfolio::connectBalance(const Entity::Balance *balance)
{
	connect(balance, &Entity::Balance::changed, [this,balance]()
	{
		for (const auto &pair : balance->getBalance())
		{
			PortfolioRow row("Bitfinex", pair.first, pair.second);
			model.changePortfolioRow(row);
		}
	});

	ui.tableView->connectBalance(balance);
}

void TablePortfolio::connectControllerOrderBook(const ControllerOrderBook *controller)
{
	connect(controller, &ControllerOrderBook::recievedOrderBook, &model, &ModelPortfolio::onOrderBookRecieved);
	connect(&model, &ModelPortfolio::requestAllOrderBook, controller, &ControllerOrderBook::requestAllOrderBook);
	ui.tableView->connectControllerOrderBook(controller);
}

void TablePortfolio::initConnect()
{
	connect(ui.currency, &QComboBox::currentTextChanged, this, &TablePortfolio::onSelectCurrency);
	connect(&model, &ModelPortfolio::changedTotal, [this](double total)
	{
		ui.total->setValue(total);
	});
}
