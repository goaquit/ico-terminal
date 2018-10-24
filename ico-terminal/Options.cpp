#include "Options.h"
#include <QSettings>
#include "WrapperDb.h"
#include "QSqlQuery"
#include "QSqlError"
#include <memory>
#include <QDebug>
#include <QVariant>

static const QString configFile("./local.ini");

Options::Options(QObject *parent)
	: QObject(parent)
{
	auto query = TerminalCore::WrapperDb::instance()->select("select notice_compare_tool_in_background();");
	
	if (query->next())
	{
		backgroundNotice = query->value(0).toBool();
	}

	delete query;
}

void Options::setChatId(qint32 v)
{
	chatId = v;	
}

qint32 Options::getChatId() const
{
	return chatId;
}

void Options::enableBackgroundNotice(bool enabled)
{
	backgroundNotice = enabled;
}

bool Options::isBackgroundNotice() const
{
	return backgroundNotice;
}

void Options::save()
{
	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("options");
		setting.setValue("telegram-chat-id", chatId);
		setting.setValue("wex-key",			 wexKey);
		setting.setValue("wex-secret",		 wexSecret);
	setting.endGroup();

	const QString sql(
		"select notice_compare_tool_enable_background(" + QString(backgroundNotice ? "true" : "false") + ");select notice_compare_tool_set_telegram(" + QString::number(chatId) + ");"
	);

	auto query = TerminalCore::WrapperDb::instance()->select(sql);

	if (query->lastError().isValid())
	{
		qDebug() << "Query error: " << query->lastError().databaseText();
	}
}
