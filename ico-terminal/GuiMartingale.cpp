#include "GuiMartingale.h"
#include "Balance.h"
#include "ControllerOrderBook.h"
#include <QDebug>

GuiMartingale::GuiMartingale(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.start,    &QPushButton::clicked, this, &GuiMartingale::start);
	connect(ui.stop,     &QPushButton::clicked, this, &GuiMartingale::stop);
	connect(ui.sell,     &QPushButton::clicked, this, &GuiMartingale::sell);
	connect(ui.pair, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),this,&GuiMartingale::changedPair);
	connect(ui.exchange, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &GuiMartingale::changedExchange);
	connect(&martingale, &Martingale::finished, this, &GuiMartingale::rest);
	connect(&martingale, &Martingale::profit, this, &GuiMartingale::onProfit);
}

void GuiMartingale::initConnect(const Entity::Balance *balance)
{
	connect(&martingale, &Martingale::buy,  balance, &Entity::Balance::buy);
	connect(&martingale, &Martingale::sell, balance, &Entity::Balance::sell);
	
	connect(balance, &Entity::Balance::sold,   &martingale, &Martingale::onSell);
	connect(balance, &Entity::Balance::bought, &martingale, &Martingale::onBuy);

	connect(balance, &Entity::Balance::error, this, &GuiMartingale::onError);

	connect(balance, &Entity::Balance::bought, this, &GuiMartingale::onBuy);
	connect(balance, &Entity::Balance::sold,   this, &GuiMartingale::onSell);

	connect(ui.stopLossValue,   QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GuiMartingale::onStopLossChangedValue);
	connect(ui.stopLossPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GuiMartingale::onStopLossChangedPercent);
}

void GuiMartingale::onChangedAsk(double price, const QString & index)
{
	const auto exchange = ui.exchange->currentText().trimmed();
	const auto currentIndex = exchange + "_" + ui.pair->currentText().trimmed().toLower();

	if (currentIndex.compare(index, Qt::CaseInsensitive)) return;

	ui.bid->setValue(price);


}

void GuiMartingale::onChangedBid(double price, const QString & index)
{
	const auto exchange = ui.exchange->currentText().trimmed();
	const auto currentIndex = exchange + "_" + ui.pair->currentText().trimmed().toLower();

	if (currentIndex.compare(index, Qt::CaseInsensitive)) return;

	ui.ask->setValue(price);

	if (martingale.hasRunning() && ui.stopLossValue->value() > 0.0 && ui.stopLossValue->value() > price)
	{
		sell();
	}
}

void GuiMartingale::onError(const QString & msg)
{
	qDebug() << msg;
	ui.log->addItem(new QListWidgetItem(msg));

	if (ui.stopAndSale->isChecked())
	{
		sell();
	}
	else
	{
		stop();
	}
	
}

void GuiMartingale::onBuy(double volume, double price)
{
	const auto currentCost = volume * price;
	const auto commission = currentCost * 0.002;

	const QString msg("Buy - Price: %1 - Volume: %2 - Cost: %3 - Comission: %4");
	qDebug() << msg.arg(price, 0, 'f', 2).arg(volume, 0, 'f', 4).arg(currentCost, 0, 'f', 2).arg(commission, 0, 'f', 4);
	ui.log->addItem(new QListWidgetItem(msg.arg(price, 0, 'f', 2).arg(volume, 0, 'f', 4).arg(currentCost, 0, 'f', 2).arg(commission, 0, 'f', 4)));

	ui.price->setValue(martingale.getPrice());
	ui.volume->setValue(martingale.getVolume());
	ui.cost->setValue(martingale.getCost());
}

void GuiMartingale::onSell(double volume, double price)
{
	const auto currentCost = volume * price;
	const auto commission = currentCost * 0.002;	

	const QString msg("Sell - Price: %1 - Volume: %2 - Cost: %3 - Comission: %4");
	qDebug() << msg.arg(price,0,'f',2).arg(volume, 0, 'f', 4).arg(currentCost, 0, 'f', 2).arg(commission,0,'f',4);
	ui.log->addItem(new QListWidgetItem(msg.arg(price, 0, 'f', 2).arg(volume, 0, 'f', 4).arg(currentCost, 0, 'f', 2).arg(commission, 0, 'f', 4)));
	
	ui.price->setValue(martingale.getPrice());
	ui.volume->setValue(martingale.getVolume());
	ui.cost->setValue(martingale.getCost());
}

void GuiMartingale::onStopLossChangedValue(double value)
{
	if (value == 0.0) return;

	const auto buyPrice = ui.ask->value();

	const auto percent = (buyPrice - value) / value * 100.0;

	ui.stopLossPercent->blockSignals(true);
	ui.stopLossPercent->setValue(percent);
	ui.stopLossPercent->blockSignals(false);
}

void GuiMartingale::onStopLossChangedPercent(double percent)
{
	const auto buyPrice = ui.ask->value();

	const auto value = buyPrice - (buyPrice * (percent / 100.0));

	ui.stopLossValue->blockSignals(true);
	ui.stopLossValue->setValue(value);
	ui.stopLossValue->blockSignals(false);
}

void GuiMartingale::onProfit(double profit)
{
	QString msgProfit("Profit: %1");

	qDebug() << msgProfit.arg(profit);
	ui.log->addItem(new QListWidgetItem(msgProfit.arg(profit)));

	if ((ui.repeatProfit->isChecked()) && (profit > 0))
	{
		start();
	}
}

void GuiMartingale::timerEvent(QTimerEvent *e)
{	
	if (timerId != e->timerId()) return;

	const auto priceBuy  = ui.bid->value();
	const auto priceSell = ui.ask->value();

	if (priceBuy == 0.0)
	{
		qDebug() << "Price BID is null";
		return;
	}

	const auto commisionRate = priceBuy * 0.002;

	auto msg("Check price: (test)" + QString::number(priceBuy - commisionRate) + QString(" with ") + QString::number(martingale.getPrice()));

	qDebug() << msg;
	ui.log->addItem(new QListWidgetItem(msg));
	
	ui.price->setValue(martingale.getPrice());
	ui.volume->setValue(martingale.getVolume());
	ui.cost->setValue(martingale.getCost());

	martingale.check(priceBuy, priceSell);

	ui.log->scrollToBottom();
}

void GuiMartingale::start()
{
	qDebug() << "Starting";
	ui.log->addItem(new QListWidgetItem("Starting"));

	if (ui.startVolume->value() == 0.0 || ui.bid->value() == 0.0)
	{
		const auto msg("Start value not set or price is null");

		qDebug() << msg;
		ui.log->addItem(new QListWidgetItem(msg));

		return;
	}

	ui.start->setEnabled(false);
	ui.stop->setEnabled(true);
	ui.sell->setEnabled(true);
	
	//disable settings
	ui.exchange->setEnabled(false);
	ui.pair->setEnabled(false);
	ui.period->setEnabled(false);
	ui.startVolume->setEnabled(false);
	

	// period from ui.period (in minutes)
	const auto period = ui.period->value() * 60 * 1000;

	timerId = startTimer(period);

	const auto pair = ui.pair->currentText().trimmed();
	martingale.setPair(pair);
	martingale.start(ui.startVolume->value(), ui.bid->value());

	if (ui.stopLossPercent->value() > 0.0 && ui.stopLossValue->value() > 0.0)
	{
		onStopLossChangedPercent(ui.stopLossPercent->value());
	}
}

void GuiMartingale::stop()
{
	martingale.stop();
}

void GuiMartingale::sell()
{
	martingale.stop(ui.ask->value());
}

void GuiMartingale::rest()
{
	qDebug() << "Stopped";
	ui.log->addItem(new QListWidgetItem("Stopped"));

	ui.start->setEnabled(true);
	ui.stop->setEnabled(false);
	ui.sell->setEnabled(false);
	ui.pair->setEnabled(true);
	ui.exchange->setEnabled(true);
	ui.period->setEnabled(true);
	ui.startVolume->setEnabled(true);

	ui.price->setValue(0.0);
	ui.volume->setValue(0.0);
	ui.cost->setValue(0.0);

	if (timerId > 0)
	{
		killTimer(timerId);
	}
}

void GuiMartingale::changedPair(const QString & pair)
{
	ui.bid->setValue(0.0);
	ui.ask->setValue(0.0);
	const auto exchange = ui.exchange->currentText().trimmed();
	emit reuqestAsk(QString(exchange + "_" + pair).toLower());
}

void GuiMartingale::changedExchange(const QString & exchange)
{
	ui.bid->setValue(0.0);
	ui.ask->setValue(0.0);
	const auto pair = ui.pair->currentText().trimmed();
	emit reuqestAsk(QString(exchange + "_" + pair).toLower());
}
