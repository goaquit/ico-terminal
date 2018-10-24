#include "GraphicsTextItem.h"

#include <QPainter>

using namespace ChartCore;

GraphicsTextItem::GraphicsTextItem(QGraphicsItem *parent)
	: QGraphicsSimpleTextItem(parent) {}

//todo реаизовать заливку фона 
void ChartCore::GraphicsTextItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	QGraphicsSimpleTextItem::paint(painter, option, widget);
}

