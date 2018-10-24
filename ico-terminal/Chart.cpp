#include "Chart.h"
#include "ChartTracker.h"
#include "GraphicsTextItem.h"

#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QValueAxis>

#include <QDateTime>
#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsLayout>
#include <QGraphicsScene>
#include <QRect>
#include <QVariant>

#include <QDebug>

QT_CHARTS_USE_NAMESPACE

struct AxisRange
{
	double max = 0.0;
	double min = 0.0;
};

template<class T> void ClearVector(std::vector<T> &v)
{
	for (auto &i : v) delete i; 
	v.clear();
}

static double GetStep(const double max, const double min, const uint limit)
{
	auto diff = max - min;
	double step = 0.0;

	if (!diff)
	{
		return step;
	}

	if (diff > 1)
	{
		uint number = std::log10(diff);
		uint s = std::pow(10, number + 1);
		
		while (limit * s > diff)
		{
			s /= 2;
			const auto checkStep = std::pow(10, number);
			if (s < checkStep)
			{
				s = checkStep;
				number--;
			}
		}

		step = s;
	}
	else
	{
		uint count = 1;
		while (diff < 100)
		{
			const uint multiplier = 10;

			diff *= multiplier;
			count *= multiplier;
		}

		const uint number = std::log10(diff);
		uint s = std::pow(10, number);
		while (limit * s >= diff)
		{
			s /= 2;
		}

		step = s;
		step /= count;
	}

	return step;
}

static uint GetStepDate(const uint max, const uint min, const uint limit)
{
	std::vector<uint> steps{ 300,600,900, 1800,3600,7200,14400,28800,43200,86400,172800,345600,604800 };
	const auto diff = max - min;
	
	uint step = 0;
	for (const auto s : steps)
	{
		step = s;
		if (step * limit >= diff) break;
	}

	return step;
}

static AxisRange GetAxisRange(QAbstractAxis * a)
{
	AxisRange range;

	switch (a->type())
	{
		case QAbstractAxis::AxisTypeValue:
		{
			auto axis = qobject_cast<QValueAxis *>(a);
			range.max = axis->max();
			range.min = axis->min();
		} break;

		case QAbstractAxis::AxisTypeLogValue:
		{
			auto axis = qobject_cast<QLogValueAxis *>(a);
			range.max = axis->max();
			range.min = axis->min();
		} break;

		case QAbstractAxis::AxisTypeDateTime:
		{
			auto axis = qobject_cast<QDateTimeAxis *>(a);
			range.max = axis->max().toSecsSinceEpoch();
			range.min = axis->min().toSecsSinceEpoch();
		} break;
	}

	return range;
}

static QString GetAxisLabel(double tick, const QAbstractAxis * axis, const double step = 300)
{
	QString label;

	switch (axis->type())
	{
		case QAbstractAxis::AxisTypeValue:
		case QAbstractAxis::AxisTypeLogValue:
		{
			uint prec = 0;

			if (tick <= 100000 && tick > 1)
				prec = 2;
			else if (tick <= 1 && tick > 0.01)
				prec = 4;
			else if (tick <= 0.01 && tick > 0.0001)
				prec = 6;
			else if (tick <= 0.0001 && tick > 0.000001)
				prec = 8;


			label = QString::number(tick, 'f', prec);
		} break;

		case QAbstractAxis::AxisTypeDateTime:

			QString format = "HH:mm";

			if (step >= 86400)
			{
				format = "MMM/dd";
			}
			else if (step >= 3600)
			{
				format = "ddd HH:mm";
			}

			label = QDateTime::fromTime_t(tick).toString(format);
			break;
	}

	return label;
}

namespace ChartCore
{



Chart::Chart(QChart * parent) : QChart((parent) ? parent : new QChart) , info(new GraphicsTextItem(this))
{
	legend()->hide();
	setMargins(QMargins(0,0,0,0));
	setBackgroundRoundness(0);
	
	auto l = layout();
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);

	connect(this, &QChart::plotAreaChanged, this, &Chart::plotChanges);

	setBackgroundBrush(QBrush(Qt::black));	

	info->setPos(0, 0);
	info->setBrush(Qt::white);
	info->setZValue(DBL_MAX);
}

Chart::~Chart()
{
	delete info;
}

void Chart::setHorizontal(QAbstractAxis * axis)
{
	if (horizontal)
	{
		for (const auto value : series())
		{
			value->detachAxis(horizontal);
		}

		removeAxis(horizontal);
	}

	horizontal = axis;

	addAxis(horizontal, Qt::AlignBottom);

	for (const auto value : series())
	{
		value->attachAxis(horizontal);
	}
}

void Chart::setHorizontalType(QAbstractAxis::AxisType type)
{
	if (horizontal)
	{
		removeAllSeries();
		removeAxis(horizontal);
		horizontal->deleteLater();
	}

	switch (type)
	{
	case QtCharts::QAbstractAxis::AxisTypeNoAxis:
		horizontal = nullptr;
		break;
	case QtCharts::QAbstractAxis::AxisTypeValue:
		horizontal = new QValueAxis(this);
		break;
	case QtCharts::QAbstractAxis::AxisTypeDateTime:
		horizontal = new QDateTimeAxis(this);
		break;
	case QtCharts::QAbstractAxis::AxisTypeLogValue:
		horizontal = new QLogValueAxis(this);
		break;
	}

	addAxis(horizontal, Qt::AlignBottom);
	horizontal->hide();
}

void Chart::setInfo(const QString & infoText)
{
	info->setText(infoText);
}

void Chart::setOffsetRight(qreal v)
{
	changeOffset = true;

	auto m = margins();
	m.setRight(v);
	setMargins(m);
}

void Chart::setRangeHorizontal(const QVariant &max, const QVariant &min)
{
	horizontal->setRange(min, max);

	QFontMetrics fm(font());

	auto m = margins();
	m.setBottom(fm.height());
	setMargins(m);

	drawGrid();
}


QAbstractAxis * Chart::getHorizontal() const
{
	return horizontal;
}

QDateTime Chart::getMaxDateHorizontal() const
{
	QDateTime result;

	switch (horizontal->type())
	{
		case QAbstractAxis::AxisTypeDateTime :
			result = qobject_cast<QDateTimeAxis *>(horizontal)->max();
		break;
	}

	return result;
}

QDateTime Chart::getMinDateHorizontal() const
{
	QDateTime result;

	switch (horizontal->type())
	{
	case QAbstractAxis::AxisTypeDateTime:
		result = qobject_cast<QDateTimeAxis *>(horizontal)->max();
		break;
	}

	return result;
}

double Chart::getOffsetTop() const
{
	return margins().top();
}

double Chart::getOffsetRight() const
{
	return margins().right();
}

void Chart::scrollTo(const QVariant & value, Qt::Orientation orientation)
{
	const auto axis = (orientation == Qt::Vertical) ? vertical : horizontal;

	const auto scrolling = (orientation == Qt::Vertical) ? plotArea().height() : plotArea().width();

	qreal shift = 0.0;
	
	switch (axis->type())
	{
		case QAbstractAxis::AxisTypeDateTime :
		{
			const auto range = GetAxisRange(axis);
			const uint diff = range.max - range.min;
			const uint to = value.toUInt();
			if (to > range.max)
			{
				shift = (scrolling / diff) * (to - range.max);
			}
			
		} break;
	}

	if (orientation == Qt::Vertical)
	{
		scroll(0, shift);
	}
	else
	{
		scroll(shift, 0);
	}

	drawGrid();
}

void Chart::setVerticalType(QAbstractAxis::AxisType type)
{
	if (vertical)
	{
		removeAllSeries();
		removeAxis(vertical);
		vertical->deleteLater();		
	}

	switch (type)
	{
		case QtCharts::QAbstractAxis::AxisTypeNoAxis:
			vertical = nullptr;
			break;
		case QtCharts::QAbstractAxis::AxisTypeValue:
			vertical = new QValueAxis(this);
			break;
		case QtCharts::QAbstractAxis::AxisTypeDateTime:
			vertical = new QDateTimeAxis(this);
			break;
		case QtCharts::QAbstractAxis::AxisTypeLogValue:
			vertical = new QLogValueAxis(this);
			break;
	}

	addAxis(vertical, Qt::AlignLeft);
	vertical->hide();
}

void Chart::setVerticalTick(uint v)
{
	maxVerticalTickCount = v;
}

void Chart::setVerticalTickValue(std::vector<double> &values)
{
	ticks = values;
}

void Chart::plotChanges(const QRectF &)
{
	drawGrid();
}

bool Chart::addSeries(QAbstractSeries * series)
{
	if (!vertical || !horizontal)
	{
		return false;
	}

	QChart::addSeries(series);
	
	series->attachAxis(vertical);
	series->attachAxis(horizontal);

	return true;
}

void Chart::addTracker(const ChartTracker * tracker)
{
	tracker->addToScene(scene());
}

void Chart::changeTrackerValue(ChartTracker * tracker, QVariant value)
{
	if (!vertical || !horizontal) return;

	auto abstractAxis = horizontal;
	auto type = horizontal->type();

	if (tracker->getOrientation() == Qt::Horizontal)
	{
		abstractAxis = vertical;
		type = vertical->type();		
	}

	const auto range = GetAxisRange(abstractAxis);

	QLineF line;

	double tick = range.max;
	switch (abstractAxis->type())
	{
		case QAbstractAxis::AxisTypeLogValue:
		case QAbstractAxis::AxisTypeValue:
			tick = value.toDouble();
			break;

		case QAbstractAxis::AxisTypeDateTime:
		
			tick = (value.toULongLong() > range.max) ?
				range.min : (value.toULongLong() < range.min) ?
				range.max : range.min + (range.max - value.toULongLong());
		break;
	}

	const auto k = (range.max - tick) / (range.max - range.min);
	if (tracker->getOrientation() == Qt::Horizontal)
	{
		const auto y = pos().ry() + plotArea().top() + plotArea().height() * k;
		const auto x = pos().rx() + plotArea().left();
		const auto width = rect().width() - margins().left() - margins().right();
		const auto distance = width + 5;
		
		line.setP1(QPointF(x,y));
		line.setP2(QPointF(x + distance,y));
	}
	else
	{
		const auto y = pos().ry() + plotArea().top();
		const auto x = pos().rx() + plotArea().left() + plotArea().width() * k;
		const auto distance = plotArea().height();

		line.setP1(QPointF(x, y));
		line.setP2(QPointF(x, y + distance));
	}

	tracker->setLine(line);

	QString text;
	if (abstractAxis->type() != QAbstractAxis::AxisTypeDateTime)
	{
		text = GetAxisLabel(value.toDouble(), abstractAxis);
	}
	else
	{
		text = QDateTime::fromTime_t(value.toUInt()).toString("yy/MM/dd HH:mm");
	}
	tracker->setText(text);
}

void Chart::setRangeVertical(const QVariant &max, const QVariant &min)
{
	vertical->setRange(min, max);

	if (changeOffset) return;

	QFontMetrics fm(font());

	double offset = 0.0;
	switch (vertical->type())
	{
		case QAbstractAxis::AxisTypeValue: 
		case QAbstractAxis::AxisTypeLogValue: 
			offset = fm.width(GetAxisLabel(max.toDouble(),vertical));
			break;
		case QAbstractAxis::AxisTypeDateTime: 
			offset = fm.width(max.toDateTime().toString(dateFormat));
			break;
	}

	auto m = margins();
	m.setRight(offset + 6);
	setMargins(m);
}

void Chart::drawGrid()
{
	QBrush brushLabel(Qt::white);
	QPen penLine(Qt::darkGray, 1, Qt::PenStyle::DashLine);
	QPen penLineSolid(Qt::white, 1);

	ClearVector(gridItem);
	ClearVector(gridLabels);

	if (vertical && vertical->type() != QAbstractAxis::AxisTypeNoAxis)
	{
		const auto shift = 3;

		if(!ticks.size())
		{ // for vertical 
			const auto range = GetAxisRange(vertical);
			const auto step  = GetStep(range.max, range.min, maxVerticalTickCount);

			auto tick = range.min + (step - std::fmod(range.min, step));
			while (tick <= range.max)
			{
				const auto k = (range.max - tick) / (range.max - range.min);
				const auto y = plotArea().top() + plotArea().height() * k;
				const auto x = plotArea().left();
				const auto distance = plotArea().width();

				auto line = new QGraphicsLineItem(x, y, x + distance + 2 * shift, y, this);
				line->setPen(penLine);
				gridItem.push_back(line);

				auto label = new QGraphicsSimpleTextItem(this);
				QFontMetricsF fm(label->font());
				label->setPos(QPointF(x + distance + 2 * shift, y - fm.height() / 2));
				label->setText(GetAxisLabel(tick,vertical));
				label->setBrush(brushLabel);

				gridLabels.push_back(label);
				tick += step;
			}

			//auto separateLine = new QGraphicsLineItem(
			//	plotArea().left() + plotArea().width(),
			//	plotArea().top(),
			//	plotArea().left() + plotArea().width(), plotArea().top() + plotArea().height(), this);
			//separateLine->setPen(penLineSolid);
			//gridItem.push_back(separateLine);
		}
		else
		{
			const auto range = GetAxisRange(vertical);
			for (const auto tick : ticks)
			{
				if (tick > range.max || tick < range.min) continue;

				const auto k = (range.max - tick) / (range.max - range.min);
				const auto y = plotArea().top() + plotArea().height() * k;
				const auto x = plotArea().left();
				const auto distance = plotArea().width();

				auto line = new QGraphicsLineItem(x, y, x + distance + 2 * shift, y, this);
				line->setPen(penLine);
				gridItem.push_back(line);

				auto label = new QGraphicsSimpleTextItem(this);
				QFontMetricsF fm(label->font());
				label->setPos(QPointF(x + distance + 2 * shift, y - fm.height() / 2));
				label->setText(GetAxisLabel(tick, vertical));
				label->setBrush(brushLabel);

				gridLabels.push_back(label);
			}

			//auto separateLine = new QGraphicsLineItem(
			//	plotArea().left() + plotArea().width(),
			//	plotArea().top(),
			//	plotArea().left() + plotArea().width(), plotArea().top() + plotArea().height(), this);
			//separateLine->setPen(penLineSolid);
			//gridItem.push_back(separateLine);
		}
		
		{ // for horizontal
			const auto range = GetAxisRange(horizontal);
			const auto step  = GetStepDate(range.max, range.min, maxHorizontalTickCount);

			auto tick = range.max; 
			while (range.min <= tick)
			{
				const auto k = 1 - (range.max - tick) / (range.max - range.min);
				const auto y = plotArea().top();
				const auto x = plotArea().left() + plotArea().width() * k;
				const auto distance = plotArea().height();

				auto line = new QGraphicsLineItem(x, y, x, y + distance, this);
				line->setPen(penLine);
				gridItem.push_back(line);

				if (horizontalLabelEnabled)
				{
					auto label = new QGraphicsSimpleTextItem(this);
					label->setPos(QPointF(x, y + distance));
					label->setText(GetAxisLabel(tick, horizontal, step));
					label->setBrush(brushLabel);

					gridLabels.push_back(label);
				}
				
				tick -= step;
			}

			auto separateLine = new QGraphicsLineItem(
				plotArea().left(),
				plotArea().top() + plotArea().height(),
				plotArea().left() + plotArea().width() + shift, plotArea().top() + plotArea().height(), this);
			separateLine->setPen(penLineSolid);
			gridItem.push_back(separateLine);
		}
		
	}

}

void Chart::enabledHorizontalLabel(bool enabled)
{
	horizontalLabelEnabled = enabled;
	
	QFontMetricsF fm(font());
	auto mg = margins();
	mg.setBottom( 
		(enabled) ? fm.height() : 0
	);
	setMargins(mg);
}

void Chart::setOffsetTop(qreal offset)
{
	auto margin = margins();
	margin.setTop(offset);
	setMargins(margin);
}


}