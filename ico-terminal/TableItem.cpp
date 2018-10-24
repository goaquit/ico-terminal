#include "TableItem.h"
#include <QTimer>

TableItem::TableItem(int type) : QTableWidgetItem(type)
{
	QObject::connect(&timer, &QTimer::timeout, [this]()
	{
		const auto swapBackground = background().color(), swapText = foreground().color();

		setBackground(QBrush(backgroundHighlight)); setForeground(QBrush(textHighlight));

		backgroundHighlight = swapBackground; textHighlight = swapText;

		isHighlighted = !isHighlighted;

		auto f = font();
		f.setBold(isHighlighted);
		setFont(f);

		timer.stop();
	});
}

TableItem::TableItem(const QString & text, int type) : QTableWidgetItem(text, type)
{
	QObject::connect(&timer, &QTimer::timeout, [this]()
	{
		const auto swapBackground = background().color(), swapText = foreground().color();

		setBackground(QBrush(backgroundHighlight)); setForeground(QBrush(textHighlight));

		backgroundHighlight = swapBackground; textHighlight = swapText;

		isHighlighted = !isHighlighted;

		auto f = font();
		f.setBold(isHighlighted);
		setFont(f);

		timer.stop();
	});
}

TableItem::~TableItem()
{
	stoped = true;
}

void TableItem::highlight(uint msec)
{
	if (isHighlighted || msec == 0) return;

	timer.setInterval(msec);

	const auto swapBackground = background().color(), swapText = foreground().color();

	setBackground(QBrush(backgroundHighlight)); setForeground(QBrush(textHighlight));

	backgroundHighlight = swapBackground; textHighlight = swapText;

	isHighlighted = true;

	auto f = font();
	f.setBold(isHighlighted);
	setFont(f);

	timer.start();
}

void TableItem::setBackgroundHighlight(const QColor &color)
{
	backgroundHighlight = color;
}

void TableItem::setTextHighlight(const QColor &color)
{
	textHighlight = color;
}

void TableItem::init()
{
	backgroundHighlight = background().color();
	textHighlight       = foreground().color();
}
