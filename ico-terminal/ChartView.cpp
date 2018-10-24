#include "Chart.h"
#include "ChartView.h"
#include "ChartTracker.h"
#include "GuiIndicatorsList.h"

#include <ctime>
#include <math.h>

#include <QAction>
#include <QMenu>
#include <QDebug>

#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>

#include <QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>
#include <QDateTime>
#include <QTimer>

#include <QtCharts\QChart>
#include <QtCharts\QLineSeries>
#include <QtCharts\QDateTimeAxis>
#include <QtCharts\QValueAxis>
#include <QtCharts\QAreaSeries>
#include <QGraphicsLineItem>

#include <QGraphicsLayout>

#include <QSize>

static const uint IntervalLoopMsec = 10 * 1000;

QT_CHARTS_USE_NAMESPACE
using namespace ChartCore;

struct ViewRange
{
	qreal max;
	qreal min;
};

ViewRange GetChartRange(double max, double min, uint sizePercent, uint offsetPercent)
{
	ViewRange range;

	if (sizePercent == 0)
	{
		sizePercent = 100;
	}

	auto size = ((max - min) * 100) / sizePercent;

	range.max = max + size * offsetPercent * 0.01;
	range.min = range.max - size;

	return range;
}

void ComputeSize(const QSizeF &size, ChartCore::Chart *main, ChartCore::Chart *amount, std::vector<ChartCore::Chart *> &additional)
{
	      auto offset            = main->getOffsetTop();
	const auto right             = main->getOffsetRight();
	const auto additionalCount   = additional.size();
	const auto interval          = 10;
	const auto maxCount          = 5;
	const auto sizeMainPercent = (additionalCount) ? (additionalCount < 3) ? 0.6 : 0.4 : 0.9;

	auto height = size.height() * (sizeMainPercent );
	main->resize(QSizeF(size.width(), height));
	offset += height;

	// for amount chart min size 10% of size
	height = size.height() * 0.1;
	amount->setPos(0, offset);
	amount->resize(QSizeF(size.width(), height));
	amount->setOffsetTop(interval);
	amount->setOffsetRight(right);
	offset += height;

	if (!additionalCount) return;

	// for additional chart
	const auto sizeAdditionalChart = (1 - sizeMainPercent - 0.1) / ((additionalCount >= 5) ? 5 : additionalCount);
	height = size.height() * sizeAdditionalChart;
	for (auto chart : additional)
	{
		chart->setPos(0, offset);
		chart->resize(QSizeF(size.width(), height));
		chart->setOffsetTop(interval);
		chart->setOffsetRight(right);
		offset += height;
	}
}

namespace Terminal
{


ChartView::ChartView(QWidget *parent)
	: QGraphicsView(new QGraphicsScene, parent),
	axisDate(new QDateTimeAxis),
	trackerPrice(new ChartTracker)
{
	trackerPrice->setOrientation(Qt::Horizontal);
	trackerPrice->setBackgroundColor(QColor(Qt::lightGray));
	trackerPrice->setFontColor(QColor(Qt::black));
	trackerPrice->setPen(QPen(Qt::white, 1));

	setMouseTracking(true);

	axisDate->hide();

}

ChartView::~ChartView()
{

	delete trackerPrice;
	
	mainChart->deleteLater();
	amountChart->deleteLater();

	for (auto e : additionalCharts) e->deleteLater();

	for (auto &tracker : trackersDate)
	{
		delete tracker;
	}
}

QDateTime ChartView::maxDate() const
{
	return axisDate->max();
}

QDateTime ChartView::minDate() const
{
	return axisDate->min();
}

void ChartView::setMaxDate(const QDateTime &d)
{
	axisDate->setMax(d);

	if (amountChart)
	{
		amountChart->setRangeHorizontal(d, amountChart->getMinDateHorizontal());
	}

	for (auto chart : additionalCharts)
	{
		chart->setRangeHorizontal(d, chart->getMinDateHorizontal());
	}
}

void ChartView::setMinDate(const QDateTime &d)
{
	axisDate->setMin(d);

	if (amountChart)
	{
		amountChart->setRangeHorizontal(amountChart->getMaxDateHorizontal(), d);
	}

	for (auto chart : additionalCharts)
	{
		chart->setRangeHorizontal(chart->getMaxDateHorizontal(), d);
	}
}

void ChartView::setLastTradePrice(const Entity::TradePrice &tp)
{
	lastTrade = tp;
}

void ChartView::setMainSeries(QAbstractSeries *series)
{
	if (mainChart) return;

	mainChart = new Chart;

	mainChart->setHorizontal(axisDate);
	mainChart->setVerticalType(QAbstractAxis::AxisTypeValue);
	mainChart->setVerticalTick(10);

	mainChart->addSeries(series);

	scene()->addItem(mainChart);

	mainChart->addTracker(trackerPrice);

	auto dateTracker = new ChartCore::ChartTracker;
	dateTracker->setOrientation(Qt::Orientation::Vertical);
	dateTracker->setBackgroundColor(QColor(Qt::lightGray));
	dateTracker->setFontColor(QColor(Qt::black));
	dateTracker->setPen(QPen(Qt::white, 1));
	dateTracker->hide();

	mainChart->addTracker(dateTracker);

	trackersDate.push_back(dateTracker);
}

void ChartView::setAmountSeries(QAbstractSeries *series)
{
	if (amountChart) return;

	amountChart = new Chart;
	amountChart->setHorizontalType(QAbstractAxis::AxisTypeDateTime);
	amountChart->setVerticalType(QAbstractAxis::AxisTypeValue);
	amountChart->addSeries(series);
	amountChart->setVerticalTick(1);
	amountChart->setInfo("Volume");
	amountChart->enabledHorizontalLabel();

	scene()->addItem(amountChart);

	auto dateTracker = new ChartCore::ChartTracker;
	dateTracker->setOrientation(Qt::Orientation::Vertical);
	dateTracker->setBackgroundColor(QColor(Qt::lightGray));
	dateTracker->setFontColor(QColor(Qt::black));
	dateTracker->setPen(QPen(Qt::white, 1));

	amountChart->addTracker(dateTracker);

	trackersDate.push_back(dateTracker);
}

void ChartView::addToMain(QAbstractSeries *s)
{
	mainChart->addSeries(s);
}

void ChartView::addAdditionalChart(ChartCore::Chart *chart)
{
	chart->setHorizontalType(QAbstractAxis::AxisTypeDateTime);
	chart->setVerticalType(QAbstractAxis::AxisTypeValue);
	chart->setRangeHorizontal(axisDate->max(), axisDate->min());
	chart->setOffsetRight(mainChart->getOffsetRight());
	chart->enabledHorizontalLabel();

	scene()->addItem(chart);

	if (additionalCharts.size())
	{
		(*additionalCharts.rbegin())->enabledHorizontalLabel(false);
	}
	else
	{
		amountChart->enabledHorizontalLabel(false);
	}

	auto dateTracker = new ChartCore::ChartTracker;

	connect(chart, &Chart::destroyed, [this,dateTracker]()
	{
		if (!trackersDate.size()) return;
		const auto itr = std::find(trackersDate.begin(), trackersDate.end(), dateTracker);
		if (itr == trackersDate.end()) return;
		trackersDate.erase(itr);

		delete dateTracker;
	});

	dateTracker->setOrientation(Qt::Orientation::Vertical);
	dateTracker->setBackgroundColor(QColor(Qt::lightGray));
	dateTracker->setFontColor(QColor(Qt::black));
	dateTracker->setPen(QPen(Qt::white, 1));

	(*trackersDate.rbegin())->hide();

	chart->addTracker(dateTracker);

	trackersDate.push_back(dateTracker);

	additionalCharts.push_back(chart);
}

uint ChartView::getAdditionalChartCount() const
{
	return static_cast<uint>( additionalCharts.size());
}

void ChartView::scrollHorizontal(const QDateTime &date)
{
	mainChart->scrollTo(date);
	amountChart->scrollTo(date);
	for (auto indicator : additionalCharts)
	{
		indicator->scrollTo(date);
	}
}

void ChartView::scrollToLastDate()
{
	if (lastTrade.isEmpty()) return;

	mainChart->scrollTo(lastTrade.getDate());
	amountChart->scrollTo(lastTrade.getDate());

	for (auto indicator : additionalCharts)
	{
		indicator->scrollTo(lastTrade.getDate());
	}
}

void ChartView::changeSegment(int s)
{
	auto min = axisDate->max();
	setMinDate(min.addSecs(-s));
}

void ChartView::changeRangeMain(qreal max, qreal min)
{
	mainChart->setRangeVertical(max, min);
	mainChart->drawGrid();
	mainChart->changeTrackerValue(trackerPrice,lastTrade.getClosePrice());

	for (auto chart : additionalCharts)
	{
		chart->setOffsetRight(mainChart->getOffsetRight());
	}
}

void ChartView::changeRangeAmount(qreal max)
{
	amountChart->setOffsetRight(mainChart->getOffsetRight());
	amountChart->setRangeVertical(max, 0.0);
	amountChart->drawGrid();
}

void ChartView::onMainChartInfo(const QString &info)
{
	if (!mainChart) return;
	mainChart->setInfo(info);
}

void ChartView::drawCharts()
{
	ComputeSize(size(), mainChart, amountChart, additionalCharts);

	mainChart->changeTrackerValue(trackerPrice, lastTrade.getClosePrice());
}

void ChartView::onRemoveChart(const Chart *chart)
{
	for (auto iter = additionalCharts.begin(); iter != additionalCharts.end(); )
	{
		if (*iter == chart)
		{
			(*iter)->deleteLater();
			iter = additionalCharts.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	if (!additionalCharts.size())
	{
		amountChart->enabledHorizontalLabel();
	}
	else
	{
		for (auto chrt : additionalCharts)
		{
			chrt->enabledHorizontalLabel(false);
		}

		(*additionalCharts.rbegin())->enabledHorizontalLabel();
	}

	drawCharts();
}


void ChartView::resizeEvent(QResizeEvent * event)
{
	const auto size = event->size();

	if (scene())
	{
		scene()->setSceneRect(QRect(QPoint(0, 0), size));
		
		drawCharts();
	}

	QGraphicsView::resizeEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent * event)
{
	const uint date = mainChart->mapToValue(event->pos()).x() / 1000;
	
	mainChart->changeTrackerValue(trackersDate[0], date);
	amountChart->changeTrackerValue(trackersDate[1], date);

	emit requestDateInfo(date);

	if (trackersDate.size() > 2)
	{
		auto tracker = trackersDate.begin() + 2;
		auto chart = additionalCharts.begin();
		for (; tracker != trackersDate.end() && chart != additionalCharts.end(); ++tracker, ++chart)
		{
			(*chart)->changeTrackerValue(*tracker, date);
		}	
	}

	QGraphicsView::mouseMoveEvent(event);
}



}
