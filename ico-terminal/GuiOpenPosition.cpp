#include "GuiOpenPosition.h"
#include "ui_WidgetOrderLimit.h"
#include "ui_WidgetOrderMarket.h"
#include "Position.h"

void ConnectPositionToGui(GuiOpenPosition *gui,const Position *position)
{
	auto c1 = QObject::connect(position, &Position::isSuccess, [gui]()
	{
		gui->close();
	});

	auto c2 = QObject::connect(position, &Position::error, gui, &GuiOpenPosition::onError);

	QObject::connect(gui, &GuiOpenPosition::destroyed, [position, c1, c2]()
	{
		position->disconnect(c1);
		position->disconnect(c2);
	});
}

GuiOpenPosition::GuiOpenPosition(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	onOrderLimit();

	connect(ui.orderType, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this](int index) 
	{ 
		switch (index)
		{
		case 0: onOrderLimit(); break;
		case 1:
		default:
			onOrderMarket();
			break;
		}
	});
}

void GuiOpenPosition::setExchangePairs(const std::vector<ExchangePair> &exchangePair)
{
	ui.exchange->clear();
	ui.pair->clear();

	for (const auto &ep : exchangePair)
	{
		ui.exchange->addItem(ep.first.second, ep.first.first);

		connect(ui.exchange, QOverload<int>::of(&QComboBox::activated),
			[this, &ep](int index)
		{
			Q_UNUSED(index);

			ui.pair->clear();
			for (const auto &pair : ep.second)
			{
				ui.pair->addItem(std::get<1>(pair), std::get<0>(pair));
			}
		});
	}

	emit ui.exchange->activated(0);
}

void GuiOpenPosition::onNewAsk(double price, const QString &index)
{
	const auto currentIndex = (ui.exchange->currentText() + "_" + ui.pair->currentText()).toLower();
	if (index.compare(currentIndex)) return;

	ui.priceAsk->setValue(price);
}

void GuiOpenPosition::onNewBid(double price, const QString &index)
{
	const auto currentIndex = (ui.exchange->currentText() + "_" + ui.pair->currentText()).toLower();
	if (index.compare(currentIndex)) return;

	ui.priceBid->setValue(price);
}

void GuiOpenPosition::onError(const QString & msg)
{
	ui.errorLine->setText(msg);
}



void GuiOpenPosition::onOrderLimit()
{	
	if (orderType)
	{
		orderType->deleteLater();
	}

	orderType = new QWidget;
	auto prop(new  Ui::WidgetOrderLimit);
	
	prop->setupUi(orderType);
	ui.layoutOrder->addWidget(orderType);
	orderType->show();

	connect(prop->place, &QPushButton::clicked, [this, prop]()
	{
		const auto price  = prop->price->value();
		const auto volume = prop->volume->value();

		if (price == 0.0 || volume == 0.0) return;

		auto position(new Position);

		position->setPrice(price);
		position->setVolume(volume);
		position->setDateOpen(QDateTime::currentDateTimeUtc().toTime_t());
		position->setPair(ui.pair->currentText().toUpper());
		position->setExchange(ui.exchange->currentText());
		position->setTakeProfit(prop->takeProfit->value());
		position->setStopLoss(prop->stopLoss->value());
		position->setType(
			(!prop->type->currentText().compare("Sell", Qt::CaseInsensitive)) ? Position::Short : Position::Long
		);

		ConnectPositionToGui(this, position);

		emit openPosition(position);
	});


	connect(prop->price, QOverload<double>::of(&QDoubleSpinBox::valueChanged),[this, prop]	(double d) 
	{
		const auto total = d * prop->volume->value();

		prop->total->setText(QString::number(total, 'f', 5));
	});

	connect(prop->volume, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, prop](double d)
	{
		const auto total = d * prop->price->value();

		prop->total->setText(QString::number(total, 'f', 5));
	});
}

void GuiOpenPosition::onOrderMarket()
{
	if (orderType)
	{
		orderType->deleteLater();
	}

	orderType = new QWidget;
	auto prop(new  Ui::WidgetOrderMarket);
	prop->setupUi(orderType);
	ui.layoutOrder->addWidget(orderType);
	orderType->show();

	connect(prop->buy, &QPushButton::clicked, [this, prop]()
	{
		const auto price = ui.priceAsk->value();
		const auto volume = prop->volume->value();

		if (price == 0.0 || volume == 0.0) return;

		auto position(new Position);

		position->setPrice(price);
		position->setVolume(volume);
		position->setDateOpen(QDateTime::currentDateTimeUtc().toTime_t());
		position->setPair(ui.pair->currentText().toUpper());
		position->setExchange(ui.exchange->currentText());
		position->setTakeProfit(prop->takeProfit->value());
		position->setStopLoss(prop->stopLoss->value());
		position->setType(Position::Long);

		ConnectPositionToGui(this, position);

		emit openPosition(position, price * 1.05);
	});

	connect(prop->sell, &QPushButton::clicked, [this, prop]()
	{
		const auto price  = ui.priceBid->value();
		const auto volume = prop->volume->value();

		if (price == 0.0 || volume == 0.0) return;

		auto position(new Position);

		position->setPrice(price);
		position->setVolume(volume);
		position->setDateOpen(QDateTime::currentDateTimeUtc().toTime_t());
		position->setPair(ui.pair->currentText().toUpper());
		position->setExchange(ui.exchange->currentText());
		position->setTakeProfit(prop->takeProfit->value());
		position->setStopLoss(prop->stopLoss->value());
		position->setType(Position::Short);

		ConnectPositionToGui(this, position);

		emit openPosition(position, price * .95);
	});

	connect(prop->volume, QOverload<double>::of(&QDoubleSpinBox::valueChanged),[this, prop](double d)
	{
		const auto totalBuy  = d * ui.priceAsk->value();
		const auto totalSell = d * ui.priceBid->value();

		prop->totalBuy->setText(QString::number(totalBuy, 'f', 5));
		prop->totalSell->setText(QString::number(totalSell, 'f', 5));
	});

	auto c1 = connect(ui.priceAsk, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [prop](double price)
	{
		if (!prop) return;
		const auto total = prop->volume->value() * price;
		prop->totalBuy->setText(QString::number(total, 'f', 5));
	});

	auto c2 = connect(ui.priceBid, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [prop](double price)
	{
		if (!prop) return;
		const auto total = prop->volume->value() * price;
		prop->totalSell->setText(QString::number(total, 'f', 5));
	});

	connect(orderType, &QWidget::destroyed, [this,prop, c1, c2]()
	{
		ui.priceAsk->disconnect(c1); ui.priceBid->disconnect(c2);
		delete prop;
	});
}