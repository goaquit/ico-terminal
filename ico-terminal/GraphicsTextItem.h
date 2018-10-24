#ifndef CHART_CORE_GRAPHICS_TEXT_ITEM_H
#define CHART_CORE_GRAPHICS_TEXT_ITEM_H

#include <QGraphicsSimpleTextItem>

namespace ChartCore
{
	class GraphicsTextItem : public QGraphicsSimpleTextItem
	{
	public:
		GraphicsTextItem(QGraphicsItem *parent = nullptr);
		void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override;
	};
}

#endif // !GRAPHICS_TEXT_ITEM_H