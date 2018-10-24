#include "TablePosition.h"
#include <QMenu>
#include <QContextMenuEvent>

TablePosition::TablePosition(QWidget *parent)
	: QTableWidget(parent)
{
}

void TablePosition::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu menu(this);
	if (addPosition)
	{
		menu.addAction(addPosition);
	}

	if (closePosition)
	{
		menu.addAction(closePosition);
	}

	menu.addSeparator();

	if (modifyPosition)
	{
		menu.addAction(modifyPosition);
	}

	if (removePosition)
	{
		menu.addAction(removePosition);
	}

	menu.addSeparator();

	if (openOrderBook)
	{
		menu.addAction(openOrderBook);
	}

	if (openChart)
	{
		menu.addAction(openChart);
	}

	menu.exec(event->globalPos());
}
