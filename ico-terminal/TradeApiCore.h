#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include "json.hpp"


class TradeApiCore : public QObject
{
	Q_OBJECT

public:

	enum Exchange : uint8_t
	{
		None,
		Wex
	};

	enum Type : uint8_t
	{
		Buy,
		Sell
	};

	explicit TradeApiCore(QObject *parent = nullptr);
	~TradeApiCore() = default;

	static TradeApiCore * Get(Exchange type, QObject *parent = nullptr);

	virtual bool isAvailable() const {return true;}

	void setUrl(const QString &u) {	url.setUrl(u);	}

public slots:
	virtual void onDataRecieved(QNetworkReply *reply) {	reply->deleteLater(); }

private:
	QNetworkAccessManager manager;

protected:
	void post(const QNetworkRequest &, const QByteArray &postData);
	Exchange type = None;
	QUrl url;
};
