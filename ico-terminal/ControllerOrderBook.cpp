#include "ControllerOrderBook.h"
#include <QDataStream>
#include <QTimer>
#include <QHostAddress>
#include "Entity/Balance.h"


ControllerOrderBook::ControllerOrderBook(QObject *parent)
	: QObject(parent)
{
	connect(&socket, &QIODevice::readyRead, this, &ControllerOrderBook::onReadyRead);
	connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this, &ControllerOrderBook::onError);
	connect(&socket, &QAbstractSocket::connected, [this]() { emit isActive(true); requestAllOrderBook(); });
}

void ControllerOrderBook::onReadyRead()
{
	try
	{
	///////////////////////////////////////////////////
	
	auto data = socket.readAll();

	QDataStream stream(&data, QIODevice::ReadOnly);
	
	while (!stream.atEnd())
	{
		OrderBook orderBook;
		QString index;
		stream >> orderBook >> index;

		if (!index.size()) continue;

		emit recievedOrderBook(orderBook, index);

		storage[index] = orderBook.getLimit(1);
	}
	
	///////////////////////////////////////////////////
	}
	catch (std::bad_alloc &)
	{
		qDebug() << "Order book: Incorrect data recieved";
	}
}

void ControllerOrderBook::start(const QString & h, quint16 p)
{
	hostName = h; port = p;

	socket.connectToHost(hostName, port);
}

void ControllerOrderBook::requestOrderBook(const QString & index)
{
	if (!socket.isValid())
	{
		qDebug() << "Socket OrderBook not ready";
		return;
	}

	const QString request("request:book;arg:" + index.toLower() + ";#");
	socket.write(request.toUtf8());
}

void ControllerOrderBook::requestAllOrderBook()
{
	if (!socket.isValid())
	{
		qDebug() << "Socket OrderBook not ready";
		return;
	}

	QString request("request:storage_lite;arg:;");

	socket.write(request.toUtf8());
}

void ControllerOrderBook::onError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket OrderBook error(" << socketError << "): " << socket.errorString();

	socket.abort();

	emit isActive(false);

	QTimer::singleShot(5000, [this]()
	{
		socket.connectToHost(hostName, port);
	});
}
