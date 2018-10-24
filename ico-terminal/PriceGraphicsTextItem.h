#pragma once

#include <QGraphicsSimpleTextItem>

namespace ChartCore
{
	class PriceGraphicsTextItem : public QGraphicsSimpleTextItem
	{
		QColor colorBackground = Qt::white;
		QColor colorText       = Qt::black;
	public:
		PriceGraphicsTextItem(QGraphicsItem *parent = nullptr);
		void setColorBackground(QColor);
		void setColorText(QColor);

		void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override;
	};
}