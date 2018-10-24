#include "Balance.h"
#include "TradeApiDemo.h"



namespace Entity
{


Balance::Balance(QObject *parent): QObject(parent){}

void Balance::synchronize()
{
	if (!tradeApi)
	{
		tradeApi = uptrTradeApi(new TradeApiDemo);
		connect(tradeApi.get(), &TradeApiDemo::newBalance, this, &Balance::onGetBalance);
		connect(tradeApi.get(), &TradeApiDemo::error, this, &Balance::error);
	
		qRegisterMetaType<TradeApiCore::Type>("Type");
		connect(tradeApi.get(), &TradeApiDemo::opened, [this](double price, double volume, TradeApiCore::Type type, const QString &/*pair*/)
		{
			switch (type)
			{
			case TradeApiCore::Buy:
				emit bought(volume, price);

				break;
			case TradeApiCore::Sell:
				emit sold(volume, price);
				break;
			}
		});
	}

	tradeApi->synchronize();
	tradeApi->getInfo();
}

void Balance::onGetBalance(const QVector<QPair<QString, double>>& balanceList)
{
	for (const auto &b : balanceList)
	{
		balance[b.first] = b.second;
	}

	emit changed();
}

void Balance::buy(double volume, double price, const QString &pair)
{
	tradeApi->openOrder(price, volume, TradeApiCore::Buy, pair);
}

void Balance::sell(double volume, double price, const QString &pair)
{
	tradeApi->openOrder(price, volume, TradeApiCore::Sell, pair);	
}

}

