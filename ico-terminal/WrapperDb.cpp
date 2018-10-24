#include "WrapperDb.h"
#include <QSqlQuery>
#include <QDebug>

namespace TerminalCore
{
	bool WrapperDb::init()
	{
		QMutexLocker lock(&mtx);

		const auto result = db.open();

		if (!result)
		{
			qDebug() << db.lastError().databaseText();
		}

		return result;
	}

	bool WrapperDb::exec(QSqlQuery & query)
	{
		QMutexLocker lock(&mtx);

		if (!db.isOpen())
		{
			db.open();
		}


		return query.exec();
	}

	bool WrapperDb::exec(QSqlQuery * query)
	{
		QMutexLocker lock(&mtx);

		if (!db.isOpen())
		{
			db.open();
		}

		return exec(*query);
	}

	QSqlQuery * WrapperDb::select(const QString & sql)
	{
		QMutexLocker lock(&mtx);

		if (!db.isOpen())
		{
			db.open();
		}

		QSqlQuery *query = new QSqlQuery(db);
		query->setForwardOnly(true);
		query->exec(sql);

		return query;
	}

	QSqlError WrapperDb::lastError()
	{
		QMutexLocker lock(&mtx);

		return db.lastError();
	}

	QSqlDatabase * WrapperDb::database()
	{
		return &db;
	}

	WrapperDb::WrapperDb(QObject *parent)
		: QObject(parent),db(QSqlDatabase::addDatabase("QPSQL"))
	{
	}

	WrapperDb * WrapperDb::createInstance()
	{
		return new WrapperDb;
	}

	WrapperDb::~WrapperDb()
	{
		if (db.isOpen())
		{
			db.close();
		}
	}

	WrapperDb * WrapperDb::instance()
	{
		return Singleton<WrapperDb>::instance(WrapperDb::createInstance);
	}

	void WrapperDb::setHost(const QString & host)
	{
		QMutexLocker lock(&mtx);

		db.setHostName(host);
	}

	void WrapperDb::setDataBaseName(const QString &name)
	{
		QMutexLocker lock(&mtx);

		db.setDatabaseName(name);
	}

	void WrapperDb::setUserName(const QString &user)
	{
		QMutexLocker lock(&mtx);

		db.setUserName(user);
	}

	void WrapperDb::setPassword(const QString &password)
	{
		QMutexLocker lock(&mtx);

		db.setPassword(password);
	}
}