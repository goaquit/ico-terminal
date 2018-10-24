#pragma once

#include <QObject>
#include "TradeApiCore.h"
#include "TradeOrderInfo.h"
#include <queue>
#include <QTimer>

using PostRequest = std::pair<QNetworkRequest, QByteArray>;

class TradeApiWex : public TradeApiCore
{
	Q_OBJECT
public:
	TradeApiWex(QObject *parent = nullptr);
	TradeApiWex(const TradeApiWex &copy);

	TradeApiWex & operator=(const TradeApiWex &copy);

	void onDataRecieved(QNetworkReply *reply) override;

	bool isAvailable() const override;

	void setKey(const QString &);
	void setSecret(const QString &);

public slots:
	virtual void getInfo();
	virtual void synchronize();
	virtual void openOrder(const double &price, const double &amount, Type type, const QString &pair);
	virtual void orderInfo(uint id);
	virtual void cancelOrder(uint id);
	virtual void activeOrders();

signals:
	void synchronized();
	void newOrder(const TradeOrderInfo &);
	void canceledOrder(const TradeOrderInfo &);
	void recievedOrderInfo(const TradeOrderInfo &);
	void volumeRecieved(double);
	void isSuccess();
	void error(const QString &);
	void newBalance(const QVector<QPair<QString, double>> &);
	void newActiveOrders(const QVector<TradeOrderInfo> &);

private:
	QString key = "";
	QString secret = "";

	bool isSynch = false;

	void sendRequst(const PostRequest &request);
};



class TradeApiWexNonce
{
public:
	static TradeApiWexNonce& Instance()
	{
		static TradeApiWexNonce s;
		return s;
	}

	TradeApiWexNonce(TradeApiWexNonce const&) = delete;
	TradeApiWexNonce& operator= (TradeApiWexNonce const&) = delete;

	void setNonce(uint n) {	nonce = n;	  }
	uint getNonce() const {	return nonce; }
	
	bool		hasRequest() const					   {return !queueRequests.empty();	}
	PostRequest getRequest();
	void		addRequest(const PostRequest &request) {queueRequests.push(request);	}

	bool hasProcessing();

	inline void startProcessing() {processing = true;	}
	inline void stopProcessing()  {processing = false;	}

	inline void updateLastTime() {	lastRequestTime = std::time(nullptr);	}

private:
	TradeApiWexNonce() {};

	uint nonce = 1;

	std::queue<PostRequest> queueRequests;

	bool processing = false;

	time_t lastRequestTime = 0;
};