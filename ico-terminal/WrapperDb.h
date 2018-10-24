#pragma once

#include <QObject>
#include <QMUtex>
#include <QString>
#include "Singleton.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

namespace TerminalCore
{
	class WrapperDb : public QObject
	{
		Q_OBJECT
	public:		
		~WrapperDb();
		// Singleton methods
		static WrapperDb * instance();

		// Wrapper Db methods
		void setHost(const QString&);
		void setDataBaseName(const QString&);
		void setUserName(const QString&);
		void setPassword(const QString&);

		bool init();

		bool exec(QSqlQuery &query);
		bool exec(QSqlQuery *query);

		QSqlQuery * select(const QString &sql);

		QSqlError lastError();

		QSqlDatabase *database();

	private:
		// Singleton methods
		WrapperDb(QObject *parent = nullptr);
		static WrapperDb * createInstance();
		////////////////////

		QSqlDatabase db;
		QMutex mtx;
	};
}