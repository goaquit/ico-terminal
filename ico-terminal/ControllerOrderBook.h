#pragma once

#include <QObject>
#include <QTcpSocket>
#include <map>

// entity
#include "DepthOrder.h"

namespace Entity
{
class Balance;
}

using OrderBook = Entity::DepthOrder;

class ControllerOrderBook : public QObject
{
	Q_OBJECT

public:
	ControllerOrderBook(QObject *parent = nullptr);
	~ControllerOrderBook() = default;

	void start(const QString &host, quint16 port);

public slots:
	void requestOrderBook(const QString &index);
	void requestAllOrderBook();

signals:
	void recievedOrderBook(const OrderBook &orderBook, const QString &index);
	void isActive(bool enable);

private:
	QTcpSocket socket;

	QString hostName;
	quint16 port;

	std::map<QString, OrderBook> storage; 

private slots:
	void onReadyRead();
	void onError(QAbstractSocket::SocketError socketError);

};
