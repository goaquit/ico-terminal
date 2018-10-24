#include "PriceGraphicsTextItem.h"

#include <QPainter>

namespace ChartCore
{
	PriceGraphicsTextItem::PriceGraphicsTextItem(QGraphicsItem * parent)
		: QGraphicsSimpleTextItem(parent) {}

	void PriceGraphicsTextItem::setColorBackground(QColor c)
	{
		colorBackground = c;
	}

	void PriceGraphicsTextItem::setColorText(QColor c)
	{
		colorText = c;
	}


	void PriceGraphicsTextItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
	{
		setBrush(QBrush(colorText));
		painter->setBrush(QBrush(colorBackground));
		painter->drawRect(boundingRect());
		QGraphicsSimpleTextItem::paint(painter, option, widget);
	}
}