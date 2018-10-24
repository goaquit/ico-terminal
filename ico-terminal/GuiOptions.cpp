#include <QAbstractButton>
#include "GuiOptions.h"
#include "PropertyNotification.h"
#include "PropertyApi.h"

enum OptionType : uint
{
	Api,
	Notification
	
};

GuiOptions::GuiOptions(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle("Options");
	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui.optinonList, &QListWidget::itemClicked,  this, &GuiOptions::selectItem);
	connect(ui.buttonBox,   &QDialogButtonBox::clicked, this, &GuiOptions::onDialogButtonClicked);
}

GuiOptions::~GuiOptions()
{
}

void GuiOptions::initOptions()
{
	ui.optinonList->clear();

	auto item = new QListWidgetItem("Notification", ui.optinonList);
	item->setData(Qt::UserRole + QVariant::UInt, OptionType::Notification);

	item = new QListWidgetItem("Api", ui.optinonList);
	item->setData(Qt::UserRole + QVariant::UInt, OptionType::Api);
}

void GuiOptions::selectNotification()
{
	if (!option) return;

	if (currentProperty) currentProperty->deleteLater();

	auto notification = new PropertyNotification(this);
	notification->setOptions(option);

	connect(this, &GuiOptions::applyParameters, notification, &PropertyNotification::onApply);

	currentProperty = notification;
	ui.propLayout->addWidget(currentProperty);

	currentProperty->show();
}

void GuiOptions::selectApi()
{
	if (!option) return;

	if (currentProperty) currentProperty->deleteLater();

	auto prop = new PropertyApi(this);
	prop->setOptions(option);

	connect(this, &GuiOptions::applyParameters, prop, &PropertyApi::onApply);

	currentProperty = prop;
	ui.propLayout->addWidget(currentProperty);

	currentProperty->show();
}

void GuiOptions::setOption(Options *o)
{
	option = o;

	initOptions();
}

void GuiOptions::onDialogButtonClicked(QAbstractButton * button)
{
	switch (ui.buttonBox->standardButton(button))
	{
	case QDialogButtonBox::Apply:
		emit applyParameters();
		break;

	case QDialogButtonBox::Ok:
		emit applyParameters();
	case QDialogButtonBox::Close: 
		close(); 
		break;
	}
}

void GuiOptions::selectItem(const QListWidgetItem *item)
{
	auto type = static_cast<OptionType>(item->data(Qt::UserRole + QVariant::UInt).toUInt());

	switch (type)
	{
		case Api:				selectApi();				break;
		case Notification:		selectNotification();		break;
		
	}

	if (ui.widget->isVisible())
	{
		ui.widget->hide();
		
	}
}
