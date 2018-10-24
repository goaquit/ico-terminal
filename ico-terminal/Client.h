#pragma once

#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QMutex>
#include <queue>
#include <QAbstractSocket>

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QThread;
QT_END_NAMESPACE

namespace Loader
{


class Client : public QObject
{
	Q_OBJECT

public:
	Client(QObject *parent = nullptr);
	~Client();

	Client& operator= (const Client &copy);

	void start(const QString &hostName = "", quint16 port = 0);

	bool isValid()const;

	bool checkBlock();

signals:
	void blockReady(QByteArray &);
	void isReady();

public slots:
	void send(const QString &message);
	void onError(QAbstractSocket::SocketError socketError);

private:
	QTcpSocket * socket;
	QMutex mtx;

	std::queue<QString> requestQueue;

	QTimer blockTimer;

	QByteArray block;
	quint32 blockSize = 0;

	QString hostName = "";
	quint16 port = 0;

private slots:
	void blockClear();
	void blockRead();
};


}