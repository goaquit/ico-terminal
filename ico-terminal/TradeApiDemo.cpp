#include "TradeApiDemo.h"
#include <QDir>
#include <QFile>
#include <QDataStream>
#include "ControllerOrderBook.h"

using Balance = QPair<QString, double>;

class TradeDemoFile
{
public:
	static TradeDemoFile& Instance()
	{
		static TradeDemoFile o; return o;
	}

	TradeDemoFile(const TradeDemoFile const &) = delete;
	TradeDemoFile& operator= (const TradeDemoFile const&) = delete;



	bool appendVolume(const QString &currency, double volume)
	{
		auto list = readBalance();

		Balance balance({ currency.toLower() , volume });

		auto el = std::find(list.begin(), list.end(), balance);

		if (el == list.end()) return false;

		(*el).first = currency;
		(*el).second += volume;

		writeBalance(list);

		return true;
	}

	bool subtract(const QString &currency, double volume)
	{
		auto list = readBalance();

		Balance balance({ currency.toLower() , volume });

		auto el = std::find(list.begin(), list.end(), balance);

		if (el == list.end()) return false;

		(*el).first = currency;
		auto current = (*el).second;

		if (current < volume)
		{
			current = 0.0;
		}
		else
		{
			current -= volume;
		}

		(*el).second = current;

		writeBalance(list);

		return true;
	}

	QList<Balance> readBalance() const
	{
		QList<Balance> result;

		if (!QDir("profile").exists())
		{
			QDir().mkdir("profile");
		}

		const QString path("profile/demo");
		if (!QDir(path).exists())
		{
			QDir().mkdir(path);
		}

		const auto fn(path + "/bitfinex.data");
		QFile file(fn);

		if (!file.open(QIODevice::ReadWrite))
		{
			qDebug() << "Demo file `wex.data` not open";
			return result;
		}

		QDataStream stream(&file);

		stream >> result;

		file.close();

		return result;
	}
	bool writeBalance(const QList<Balance> &list)
	{
		QList<Balance> result;

		if (!QDir("profile").exists())
		{
			QDir().mkdir("profile");
		}

		const QString path("profile/demo");
		if (!QDir(path).exists())
		{
			QDir().mkdir(path);
		}

		const auto fn(path + "/bitfinex.data");
		QFile file(fn);

		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qDebug() << "Demo file `bitfinex.data` not open";
			return false;
		}

		QDataStream stream(&file);

		stream << list;

		file.close();

		return true;
	}

private:
	TradeDemoFile() {};
};

void TradeApiDemo::getInfo()
{
	auto balance = TradeDemoFile::Instance().readBalance();

	if (!balance.size()) synchronize();

	QVector<QPair<QString, double>> list = balance.toVector();

	emit newBalance(list);
}

void TradeApiDemo::synchronize()
{

	auto list = TradeDemoFile::Instance().readBalance();
	if (!list.size())
	{
		list.push_back({ QString("btc"),1 });
		list.push_back({ QString("usd"),100000 });
		list.push_back({ QString("ltc"),0 });
		list.push_back({ QString("eth"),0 });

		TradeDemoFile::Instance().writeBalance(list);
	}

	emit synchronized();
}

void TradeApiDemo::openOrder(const double & price, const double & amount, Type type, const QString & pair)
{
	auto balance = TradeDemoFile::Instance().readBalance();

	const auto commissionRate = 0.002; // percent

	// find currency
	Balance base; 
	Balance quote;
	for (auto first = balance.begin(); first != balance.end(); ++first)
	{
		for (auto second = balance.begin(); second != balance.end(); ++second)
		{
			if (first == second) continue;
			const auto checking = first->first + second->first;
			if (pair.compare(checking, Qt::CaseInsensitive)) continue;

			base = *first;
			quote   = *second;
			
			break;
		}
	}

	const auto cost		  = price * amount;
	const auto commission = commissionRate * cost;

	if (type == Buy)
	{
		if (quote.second < cost + commission)
		{
			emit error("Insufficient funds " + quote.first.toUpper() + " on account");
			return;
		}

		quote.second -= cost + commission;
		base.second += amount;
	}
	else
	{
		if (base.second < amount)
		{
			emit error("Insufficient funds " + base.first.toUpper() + " on account");
			return;
		}

		base.second -= amount;
		quote.second += cost - commission;
	}

	for (auto el = balance.begin(); el != balance.end(); ++el)
	{
		if (!el->first.compare(base.first,Qt::CaseInsensitive))
		{
			*el = base;
			continue;
		}

		if (!el->first.compare(quote.first, Qt::CaseInsensitive))
		{
			*el = quote;
			continue;
		}
	}
	
	TradeDemoFile::Instance().writeBalance(balance);

	emit isSuccess();
	emit volumeRecieved(amount);
	emit opened(price, amount, type, pair);

	getInfo();
}

void TradeApiDemo::orderInfo(uint id)
{
}

void TradeApiDemo::cancelOrder(uint id)
{
}

void TradeApiDemo::activeOrders()
{
}

void TradeApiDemo::initControllerOrderBook(const ControllerOrderBook *)
{

}


