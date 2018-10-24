#pragma once

#include <QObject>
#include <QSslSocket>
#include <QAbstractSocket>

class UserRegistrationClient : public QObject
{
	Q_OBJECT

public:
	explicit UserRegistrationClient(QObject *parent = nullptr);
	~UserRegistrationClient();

	void connect(const QString& host = "");

	void setPort(quint16);

public slots:
	void registration(const QString &login, const QString & password, const QString &checkword);
	void changePassword(const QString &login, const QString & password, const QString &checkword);


signals:
	void message(const QString &);
	void registerSuccess();
	void changePasswordSuccess();

private:
	QSslSocket * socket = nullptr;

private slots:
	void socketReady();
	void socketSslErrors(const QList<QSslError> &);
	void socketError(QAbstractSocket::SocketError);
	void socketRead();


private:
	QString host = "127.0.0.1";
	quint16 port = 1234;
};
