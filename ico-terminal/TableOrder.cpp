#include "TableOrder.h"
#include <QMenu>
#include <QContextMenuEvent>

TableOrder::TableOrder(QWidget *parent) : QTableWidget(parent) {}

void TableOrder::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu menu(this);

	if (closeOrder)
	{
		menu.addAction(closeOrder);
	}
	menu.exec(event->globalPos());
}
