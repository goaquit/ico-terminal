#include "ChartTracker.h"
#include "PriceGraphicsTextItem.h"


#include <QFontMetrics>
#include <QGraphicsScene>
#include <QPen>

namespace ChartCore
{


ChartTracker::ChartTracker() : 
	label(new LabelGrid),
	line(new QGraphicsLineItem)
{
	line->setZValue(UINT32_MAX);
	label->setZValue(UINT32_MAX);
}

ChartTracker::~ChartTracker()
{
	auto scene = label->scene();
	if (scene)
	{
		scene->removeItem(label);
		scene->removeItem(line);
	}
	

	delete label;
	delete line;
}

void ChartTracker::addToScene(QGraphicsScene * scene) const
{
	if (scene)
	{
		scene->addItem(line);
		scene->addItem(label);
	}
}

Qt::Orientation ChartTracker::getOrientation() const
{
	return orientation;
}

void ChartTracker::hide()
{
	label->hide();
	hideLabel = true;
}

void ChartTracker::setBackgroundColor(const QColor & color)
{
	label->setColorBackground(color);
}

void ChartTracker::setFontColor(const QColor & color)
{
	label->setColorText(color);
}

void ChartTracker::setLabelSize(qreal)
{
}

void ChartTracker::setOrientation(Qt::Orientation o)
{
	orientation = o;
}

void ChartTracker::setPen(const QPen &pen)
{
	line->setPen(pen);
}

void ChartTracker::setLine(const QLineF & v)
{
	if (!line->scene()) return;

	line->setLine(v);

	if (hideLabel) return;
	//TODO: add limit
	QFontMetrics fm(label->font());
	switch (orientation)
	{
	case Qt::Horizontal:
		label->setPos(v.x2(), v.y2() - fm.height() / 2);
		break;
	case Qt::Vertical:
	{
		const qreal x =  v.x2() - fm.width(label->text()) / 2;
		label->setPos((x < 0) ? 0 : x, v.y2());
	}
		
		break;
	}
}

void ChartTracker::setText(const QString & value)
{
	label->setText(value);
}

void ChartTracker::show()
{
	hideLabel = false;
	setLine(line->line());
	label->show();
}

}