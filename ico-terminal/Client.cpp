#include "Client.h"
#include "Order.h"

#include <QTcpSocket>
#include <QDataStream>
#include <QThread>
#include <QHostAddress>

static const uint defaultBlockLife = 100; 

namespace Loader
{

Client::Client(QObject *parent)
	: QObject(parent), socket(new QTcpSocket(parent))
{
	QObject::connect(socket,      &QTcpSocket::readyRead, this, &Client::blockRead);
	QObject::connect(socket,      &QTcpSocket::hostFound, this, &Client::isReady);
	QObject::connect(&blockTimer, &QTimer::timeout,       this, &Client::blockClear);
	QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::onError);

	blockTimer.setInterval(defaultBlockLife);
}

Client::~Client()
{
	socket->disconnect();
	socket->deleteLater();
}

Client& Client::operator=(const Client & copy)
{
	hostName = copy.hostName; port = copy.port;

	return *this;
}

void Client::start(const QString &h, quint16 p)
{
	if (h.size())
	{
		hostName = h;
	}
	if (p)
	{
		port = p;
	}
	 
	socket->connectToHost(hostName, port);
}

bool Client::isValid() const
{
	return socket->isValid();
}

bool Client::checkBlock()
{
	return false;
}

void Client::onError(QAbstractSocket::SocketError socketError)
{
	Q_UNUSED(socketError);
	socket->abort();
	QTimer::singleShot(5000, [this]()
	{
		socket->connectToHost(hostName, port);
	});
}

void Client::send(const QString & message)
{
	QByteArray blockSend;
	QDataStream sendStream(&blockSend, QIODevice::ReadWrite);

	sendStream.writeRawData(message.toStdString().c_str(), message.size());

	socket->write(blockSend);
}

//TODO: need refactoring
void Client::blockRead()
{
	while (!socket->atEnd())
	{
		auto   size = blockSize;
		quint8 marker = 0;

		auto preambulaSize = sizeof(size) + sizeof(marker);

		QDataStream reader(&block, QIODevice::ReadOnly);

		if (!blockSize)
		{
			// read preambula data
			block = socket->read(preambulaSize);

			reader >> size >> marker;

			// check size end
			if (marker != quint8(0xFF))
			{
				blockClear();
				continue;
			}

			// read block
			blockSize = size;

			const auto bytesToRead = (socket->bytesAvailable() > size) ? size : socket->bytesAvailable();

			block.append(socket->read(bytesToRead));
			
			quint16 endFlag = 0;
			reader.device()->seek(block.size() - sizeof(endFlag));
			reader >> endFlag;

			// check end block
			if ((block.size() - preambulaSize) == size && endFlag == quint16(0xFFFF))
			{
				emit blockReady(block);
				blockClear();
			}
		}
		else
		{

			auto sizeDif = blockSize - (block.size() - preambulaSize);

			if (!sizeDif)
			{
				emit blockReady(block);
				blockClear();
				continue;
			}
			
			const auto bytesToRead = (socket->bytesAvailable() > sizeDif) ? sizeDif : socket->bytesAvailable();


			block.append(socket->read(bytesToRead));

			
			quint16 endFlag = 0;
			reader.device()->seek(block.size() - sizeof(endFlag));
			reader >> endFlag;

			// check end block
			if ((block.size() - preambulaSize) == size && endFlag == quint16(0xFFFF))
			{
				emit blockReady(block);
				blockClear();
			}
		}

		if ( (block.size() - preambulaSize) >= blockSize)
		{
			blockClear();
		}
	}

}



void Client::blockClear()
{
	block.clear();
	blockSize = 0;

	//blockTimer.stop();
}


}