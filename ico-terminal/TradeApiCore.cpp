#include "TradeApiCore.h"
#include "TradeApiWex.h"
#include <QNetworkRequest>
#include <QDebug>

TradeApiCore::TradeApiCore(QObject *parent)	: QObject(parent)
{
	QObject::connect(&manager, &QNetworkAccessManager::finished, this, &TradeApiCore::onDataRecieved);
}
TradeApiCore * TradeApiCore::Get(Exchange t, QObject *parent)
{
	TradeApiCore *result = nullptr;

	switch (t)
	{
	case TradeApiCore::Wex:
		result = new TradeApiWex(parent);
		break;
	case TradeApiCore::None:
	default:
		result = new TradeApiCore(parent);
		break;
	}

	return result;
}
void TradeApiCore::post(const QNetworkRequest &r, const QByteArray &postData)
{
	if (!isAvailable())
	{
		qDebug() << "Trade API not ready!";
		return;
	}
	manager.post(r, postData);
}