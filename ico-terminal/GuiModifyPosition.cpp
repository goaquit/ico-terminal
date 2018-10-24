#include "GuiModifyPosition.h"
#include "Position.h"


GuiModifyPosition::GuiModifyPosition(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	connect(ui.modify, &QPushButton::clicked, this, &GuiModifyPosition::modify);
}
void GuiModifyPosition::onNewBid(double v, const QString &index)
{
	if (!position || (index.compare((position->getExchange() + "_" + position->getPair()).toLower()) != 0 ) ) return;

	ui.priceBid->setValue(v);
}

void GuiModifyPosition::onNewAsk(double v, const QString &index)
{
	if (!position || (index.compare((position->getExchange() + "_" + position->getPair()).toLower()) != 0)) return;

	ui.priceAsk->setValue(v);
}

void GuiModifyPosition::readParameters()
{
	if (!position) return;

	ui.takeProfit->setValue(position->getTakeProfite());
	ui.stopLoss->setValue(position->getStopLoss());
}

void GuiModifyPosition::modify()
{
	if (!position) return;

	position->setTakeProfit(ui.takeProfit->value());
	position->setStopLoss(ui.stopLoss->value());

	emit position->requestToSave();
	emit modifyed();

	close();
}
