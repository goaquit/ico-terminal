#include "TradeApiWex.h"
#include <QUrlQuery>
#include <QMessageAuthenticationCode>
#include <QUrl>
#include <QRegExp>

using json = nlohmann::json;

static bool CheckWexNonce(const QByteArray &data)
{
	// invalid nonce parameter; on key:6, you sent:'1', you should send:7

	bool result = false;
	const QString reg(R"(you sent:\s*[''`]?(\d+)[`'`]?.*you should send:\s*[''`]?(\d+)[`'`]?)");

	QRegExp rx(reg);
	result = rx.indexIn(QString(data)) != -1;
	if (result) 
	{
		const uint expected = rx.cap(2).toUInt();
		TradeApiWexNonce::Instance().setNonce(expected);
	}

	return result;
}

TradeApiWex::TradeApiWex(QObject * parent) : TradeApiCore(parent)
{
	type = Wex;
	setUrl("https://wex.nz/tapi");
}

TradeApiWex::TradeApiWex(const TradeApiWex &copy)
{
	*this = copy;
}

TradeApiWex & TradeApiWex::operator=(const TradeApiWex & copy)
{
	key = copy.key;
	secret = copy.secret;
	type = copy.type;
	url = copy.url;
	isSynch = copy.isSynch;

	return *this;
}

void TradeApiWex::onDataRecieved(QNetworkReply * reply)
{
	if (TradeApiWexNonce::Instance().hasRequest())
	{
		sendRequst(TradeApiWexNonce::Instance().getRequest());
	}
	else
	{
		TradeApiWexNonce::Instance().stopProcessing();
	}

	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << "Reply has error : " << reply->error();

		reply->deleteLater();

		return;
	}

	const auto data(reply->readAll());
	const auto query = reply->url().query();
	reply->deleteLater();

	try
	{

	auto jsonData = json::parse(data.toStdString().c_str());
	if (jsonData.find("success") == jsonData.end())
	{
		qDebug() << "Incorrect reply";
		return;
	}

	auto success = static_cast<bool>(jsonData["success"].get<int>());

	if (!success)
	{
		if (CheckWexNonce(data))
		{
			isSynch = true;
			qDebug() << "Nonce synchronized";
			emit synchronized();

			return;
		}

		emit error(QString(jsonData["error"].get <std::string>().c_str()));

		return;
	}

	const QString reg(R"(method=(\w+))");
	QRegExp rx(reg);
	if (rx.indexIn(query) == -1)
	{
		qDebug() << "Request not found";
		return;
	}
	const auto method = rx.cap(1);

	if (!method.compare("getInfo", Qt::CaseInsensitive))
	{
		//{
		//	"success":1,
		//		"return" : {
		//			"funds":{
		//				"usd":325,
		//				"btc" : 23.998,
		//				"ltc" : 0,
		//				...
		//			}
		//	}
		//}

		try
		{
			QVector<QPair<QString, double>> balance;
			const auto funds = jsonData["return"]["funds"];
			for (auto fund = funds.begin(); fund != funds.end(); ++fund)
			{
				const auto key   = fund.key();
				const auto value = fund.value().get<double>();

				balance.push_back({ QString(key.c_str()), value });
			}

			emit newBalance(balance);
		}
		catch (nlohmann::json::exception e)
		{
			qDebug() << e.what();
		}
		
	}
	else if (!method.compare("Trade", Qt::CaseInsensitive))
	{
		uint orderId = jsonData["return"]["order_id"].get<uint>();

		if (orderId > 0)
		{
			auto order = TradeOrderInfo(orderId);
			order.setStartAmount(jsonData["return"]["remains"].get<double>());

			emit newOrder(order);
		}

		emit isSuccess();

		const auto volume = jsonData["return"]["received"].get<double>();
		if(volume != 0.0) emit volumeRecieved(volume);
	}
	else if (!method.compare("OrderInfo", Qt::CaseInsensitive))
	{
		try
		{
			for (auto orderJson = jsonData["return"].begin(); orderJson != jsonData["return"].end(); ++orderJson)
			{
				const auto k = orderJson.key();

				const auto pair				 = jsonData["return"][k]["pair"].get<std::string>();
				const auto t				 = jsonData["return"][k]["type"].get<std::string>();
				const auto startAmount		 = jsonData["return"][k]["start_amount"].get<double>();
				const auto amount			 = jsonData["return"][k]["amount"].get<double>();
				const auto rate				 = jsonData["return"][k]["rate"].get<double>();
				const auto timestamp_created = jsonData["return"][k]["timestamp_created"].get<time_t>();
				const auto status			 = jsonData["return"][k]["status"].get<uint>();

				emit recievedOrderInfo(TradeOrderInfo(QString(k.c_str()).toUInt(),QString(pair.c_str()),startAmount,amount,rate,timestamp_created,
					!t.compare("sell") ? TradeOrderInfo::Type::Sell : TradeOrderInfo::Type::Bye, TradeOrderInfo::Status(status))
				);
			}
		}
		catch (json::exception e)
		{
			qDebug() << "Order info: " << e.what();
		}
	}
	else if (!method.compare("CancelOrder", Qt::CaseInsensitive))
	{
		uint orderId = jsonData["return"]["order_id"].get<uint>();

		if (orderId > 0)
		{
			auto order = TradeOrderInfo(orderId);

			emit canceledOrder(order);
		}
	}
	else if (!method.compare("ActiveOrders", Qt::CaseInsensitive))
	{
		
			QVector<TradeOrderInfo> activeOrders;

			for (auto orderJson = jsonData["return"].begin(); orderJson != jsonData["return"].end(); ++orderJson)
			{
				const auto k = orderJson.key();

				const auto pair				 = jsonData["return"][k]["pair"].get<std::string>();
				const auto t				 = jsonData["return"][k]["type"].get<std::string>();
				const auto startAmount		 = 0.0;
				const auto amount			 = jsonData["return"][k]["amount"].get<double>();
				const auto rate				 = jsonData["return"][k]["rate"].get<double>();
				const auto timestamp_created = jsonData["return"][k]["timestamp_created"].get<time_t>();
				const auto status			 = jsonData["return"][k]["status"].get<uint>();

				activeOrders.push_back(
					TradeOrderInfo(QString(k.c_str()).toUInt(), QString(pair.c_str()), startAmount, amount, rate, timestamp_created,
						!t.compare("sell") ? TradeOrderInfo::Type::Sell : TradeOrderInfo::Type::Bye, TradeOrderInfo::Status(status))
				);
			}

			emit newActiveOrders(activeOrders);
		
	}
	else
	{
		qDebug() << data;
	}

	}
	catch (json::exception e)
	{
		qDebug() << "Order info: " << e.what();
	}
}

bool TradeApiWex::isAvailable() const
{
	return key.size() > 0 && secret.size() > 0 && isSynch;
}

void TradeApiWex::setKey(const QString &v)
{
	key = v;
}

void TradeApiWex::setSecret(const QString &v)
{
	secret = v;
}

void TradeApiWex::synchronize()
{
	isSynch = true;
	getInfo();
	isSynch = false;
}

void TradeApiWex::openOrder(const double & price, const double & amount, Type type, const QString &pair)
{
	if (!isAvailable()) return;

	auto nonce = TradeApiWexNonce::Instance().getNonce();

	QUrlQuery query;
	query.addQueryItem("nonce",		QString::number(nonce));
	query.addQueryItem("method",	"Trade");
	query.addQueryItem("pair",		pair);
	query.addQueryItem("type",		(type == Sell) ? "sell" : "buy");
	query.addQueryItem("rate",		QString::number(price));
	query.addQueryItem("amount",	QString::number(amount));

	const auto postData = query.toString().toUtf8();

	const auto sign = QMessageAuthenticationCode::hash(
		postData, secret.toUtf8(), QCryptographicHash::Sha512
	).toHex();


	auto urlRequest = url;
	urlRequest.setQuery("method=Trade");
	QNetworkRequest request(urlRequest);
	request.setRawHeader("Key", key.toUtf8());
	request.setRawHeader("Sign", sign);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	TradeApiWexNonce::Instance().setNonce(++nonce);

	sendRequst({ request, postData });
}

void TradeApiWex::orderInfo(uint id)
{
	if (!isAvailable()) return;

	auto nonce = TradeApiWexNonce::Instance().getNonce();

	QUrlQuery query;
	query.addQueryItem("nonce", QString::number(nonce));
	query.addQueryItem("method", "OrderInfo");
	query.addQueryItem("order_id", QString::number(id));

	const auto postData = query.toString().toUtf8();

	const auto sign = QMessageAuthenticationCode::hash(
		postData, secret.toUtf8(), QCryptographicHash::Sha512
	).toHex();

	auto urlRequest = url;
	urlRequest.setQuery("method=OrderInfo");
	QNetworkRequest request(urlRequest);
	request.setRawHeader("Key", key.toUtf8());
	request.setRawHeader("Sign", sign);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	TradeApiWexNonce::Instance().setNonce(++nonce);

	sendRequst({ request, postData });
}

void TradeApiWex::cancelOrder(uint id)
{
	if (!isAvailable()) return;

	auto nonce = TradeApiWexNonce::Instance().getNonce();

	QUrlQuery query;
	query.addQueryItem("nonce", QString::number(nonce));
	query.addQueryItem("method", "CancelOrder");
	query.addQueryItem("order_id", QString::number(id));

	const auto postData = query.toString().toUtf8();

	const auto sign = QMessageAuthenticationCode::hash(
		postData, secret.toUtf8(), QCryptographicHash::Sha512
	).toHex();

	auto urlRequest = url;
	urlRequest.setQuery("method=CancelOrder");
	QNetworkRequest request(urlRequest);
	request.setRawHeader("Key", key.toUtf8());
	request.setRawHeader("Sign", sign);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	TradeApiWexNonce::Instance().setNonce(++nonce);

	sendRequst({ request, postData });
}

void TradeApiWex::activeOrders()
{
	if (!isAvailable()) return;

	auto nonce = TradeApiWexNonce::Instance().getNonce();

	QUrlQuery query;
	query.addQueryItem("nonce", QString::number(nonce));
	query.addQueryItem("method", "ActiveOrders");

	const auto postData = query.toString().toUtf8();

	const auto sign = QMessageAuthenticationCode::hash(
		postData, secret.toUtf8(), QCryptographicHash::Sha512
	).toHex();

	auto urlRequest = url;
	urlRequest.setQuery("method=ActiveOrders");
	QNetworkRequest request(urlRequest);
	request.setRawHeader("Key", key.toUtf8());
	request.setRawHeader("Sign", sign);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	TradeApiWexNonce::Instance().setNonce(++nonce);

	sendRequst({ request, postData });
}

void TradeApiWex::sendRequst(const PostRequest & request)
{
	if (TradeApiWexNonce::Instance().hasProcessing())
	{
		TradeApiWexNonce::Instance().addRequest(request);
	}
	else
	{
		TradeApiWexNonce::Instance().updateLastTime();
		TradeApiWexNonce::Instance().startProcessing();
		post(request.first, request.second);
	}
}

void TradeApiWex::getInfo()
{
	if (!isAvailable()) return;

	auto nonce = TradeApiWexNonce::Instance().getNonce();

	QUrlQuery query;
	query.addQueryItem("nonce", QString::number(nonce));
	query.addQueryItem("method", "getInfo");

	const auto postData = query.toString().toUtf8();

	const auto sign = QMessageAuthenticationCode::hash(
		postData, secret.toUtf8(), QCryptographicHash::Sha512
	).toHex();

	auto urlRequest = url;
	urlRequest.setQuery("method=getInfo");
	QNetworkRequest request(urlRequest);
	request.setRawHeader("Key", key.toUtf8());
	request.setRawHeader("Sign", sign);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	TradeApiWexNonce::Instance().setNonce(++nonce);

	sendRequst({ request, postData });
}

PostRequest TradeApiWexNonce::getRequest()
{
	PostRequest result(queueRequests.front());
	queueRequests.pop(); 
	
	lastRequestTime = std::time(nullptr); 
	
	return result;
}

bool TradeApiWexNonce::hasProcessing()
{
	const auto availableWaitTime = 2;

	const auto diff = std::time(nullptr) - lastRequestTime;
	return processing && diff <= availableWaitTime;
}
