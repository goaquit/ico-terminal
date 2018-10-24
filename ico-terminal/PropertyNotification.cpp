#include "PropertyNotification.h"
#include "Options.h"
#include <QDebug>

PropertyNotification::PropertyNotification(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

void PropertyNotification::onApply()
{
	const auto telegramChatId = ui.telegramChatId->text().toLong();

	options->setChatId(telegramChatId);
	options->enableBackgroundNotice(ui.checkBoxBackgroundService->isChecked());
	options->save();
}

void PropertyNotification::setOptions(Options *o)
{
	options = o;

	const auto chatId = options->getChatId();

	if (chatId)
	{
		ui.telegramChatId->setText(QString::number(chatId));
	}
	
	ui.checkBoxBackgroundService->setChecked(options->isBackgroundNotice());
}
