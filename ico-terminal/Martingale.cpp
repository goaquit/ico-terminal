#include "Martingale.h"
#include <cmath>
#include <QDebug>

Martingale::Martingale(QObject *parent)
	: QObject(parent)
{
}

void Martingale::start(double v, double p)
{
	isRunning = isActiveOrder = true;
	emit buy(v, p, pair);
}

void Martingale::setPair(const QString & p)
{
	pair = p;
}

void Martingale::stop()
{
	clear();
}

void Martingale::stop(double p)
{
	emit sell(volume, p, pair);
}

void Martingale::check(double buyPrice, double sellPrice)
{
	if (!isActive())
	{
		qDebug() << "Martingale not running!";
	}

	const auto commision = sellPrice * commissionRate;

	if (sellPrice - commision > price)
	{
		isActiveOrder = true;
		emit sell(volume, sellPrice, pair);
	}
	else
	{
		processingNewMiddlePrice(buyPrice);		
	}
}

void Martingale::onBuy(double v, double p)
{
	if (!isRunning) return;

	static auto costBuy = 0.0;

	const auto precision = (int)std::pow(10, 4);

	costBuy += std::ceil(v * p * precision) / (precision);

	const auto commission = commissionRate * costBuy;

	costBuy += commission;

	if (costBuy < cost)
	{
		const auto newCost    = std::ceil((cost - costBuy) * precision) / precision;
		const auto tmp        = (newCost + commissionRate * newCost) / p;
		const auto newVolume  = std::ceil(tmp * precision) / precision;

		emit buy(newVolume, p, pair);

		return; 
	}

	isActiveOrder = false;

	volume += std::ceil(v * precision) / precision;
	cost   += costBuy;

	price   = std::ceil((cost / volume) * precision) / precision;

	costBuy = 0.0;
}

void Martingale::onSell(double valueSell, double priceSell)
{
	if (!isRunning) return;
	isActiveOrder = false;

	const auto precision = (int)std::pow(10, 4);
	const auto newCost = std::ceil((valueSell * priceSell) * precision) / precision;
	const auto commission = commissionRate * newCost;

	const auto prft = newCost - cost - commission;

	clear();

	emit profit(prft);
}

void Martingale::processingNewMiddlePrice(double p)
{
	const auto precision = (int)std::pow(10, 4);
	// new order
	const auto newCost   = std::ceil(cost * 2 * precision) / precision;
	const auto commission = commissionRate * newCost;

	const auto tmp       = (newCost + commission) / p;
	const auto newVolume = std::ceil(tmp * precision) / precision;

	// buying new volume
	isActiveOrder = true;
	emit buy(newVolume, p, pair);
}

void Martingale::clear()
{
	isRunning = false;
	volume = 0.0; price = 0.0; cost = 0.0;

	emit finished();
}
