#include "PropertyApi.h"
#include "Options.h"

PropertyApi::PropertyApi(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}
void PropertyApi::setOptions(Options * option)
{
	options = option;

	ui.key->setText(option->getWexKey());
	ui.secret->setText(option->getWexSecret());
}
void PropertyApi::onApply()
{
	options->setWexKey(		ui.key->text().trimmed()	);
	options->setWexSecret(	ui.secret->text().trimmed()	);
	options->save();
}
