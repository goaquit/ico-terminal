#include "UserRegistrationClient.h"

UserRegistrationClient::UserRegistrationClient(QObject *parent)
	: QObject(parent), socket(new QSslSocket(this))
{
	QObject::connect(socket, &QSslSocket::encrypted, this, &UserRegistrationClient::socketReady);
	QObject::connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(socketSslErrors(const QList<QSslError> &)));
	QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
	QObject::connect(socket, &QIODevice::readyRead, this, &UserRegistrationClient::socketRead);
}

UserRegistrationClient::~UserRegistrationClient()
{
	socket->close();
}

void UserRegistrationClient::connect(const QString & h)
{
	if (!h.isEmpty())
	{
		host = h;
	}

	//socket->connectToHostEncrypted("192.168.1.49", 1234);
	socket->connectToHostEncrypted(host, port);
	socket->ignoreSslErrors();
}

void UserRegistrationClient::setPort(quint16 p)
{
	port = p;
}

void UserRegistrationClient::socketReady()
{
}

void UserRegistrationClient::socketSslErrors(const QList<QSslError> &errors)
{
	qDebug() << "Have ssl error;";
	Q_UNUSED(errors);
}

void UserRegistrationClient::socketError(QAbstractSocket::SocketError error)
{
	qDebug() << "Error socket: " << error << socket->errorString();
}

void UserRegistrationClient::socketRead()
{
	if (!socket) return;

	QString data(socket->readAll());

	qDebug() << "Socket say: " << data;

	QRegExp rxlen("request:(\\w+);success:(\\d+);message:(.+);");
	if (rxlen.indexIn(data) < 0)
	{
		message("Request not identified.");
		return;
	}

	QString request(rxlen.cap(1));
	QString msg(rxlen.cap(3));

	emit message(msg);

	if (!request.compare("register", Qt::CaseInsensitive))
	{
		emit registerSuccess();
	}
	else if (!request.compare("changepassword", Qt::CaseInsensitive))
	{
		emit changePasswordSuccess();
	}
}

void UserRegistrationClient::registration(const QString & login, const QString & password, const QString & checkword)
{
	socket->write(QString("request:register;login:" + login + ";password:" + password + ";checkword:" + checkword + ";").toUtf8());
}

void UserRegistrationClient::changePassword(const QString & login, const QString & password, const QString & checkword)
{
	socket->write(QString("request:changepassword;login:" + login + ";password:" + password + ";checkword:" + checkword + ";").toUtf8());
}
