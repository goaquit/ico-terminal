#include "ChartController.h"
#include "TradeLoad.h"
#include "AbstractTradeLoader.h"

#include <QDateTime>

#include <QDebug>
#include <QMetaType>

#include <algorithm>

using namespace QtCharts;

Terminal::ChartController::ChartController(QObject *parent)
	: QObject(parent),
	series(new QtCharts::QLineSeries),
	seriesPriceLevel(new QtCharts::QLineSeries),
	chartTrades(new QChart),
	axisX(new QDateTimeAxis),
	axisY(new QValueAxis)
{
	
}

Terminal::ChartController::~ChartController()
{
	delete chartTrades;
}

void Terminal::ChartController::run()
{
	firstLoad();
	update();
}

void Terminal::ChartController::firstLoad()
{
	
	time_t curretTime = std::time(NULL);
	// старшая граница интервала
	auto endTime = curretTime - (curretTime % segment);

	auto endDate = QDateTime::fromTime_t(endTime).toUTC();

	// младшая граница интервала
	auto startDate = endDate.addSecs(-interval);

	double lastPrice = 0.0;


	series->append(startDate.toMSecsSinceEpoch(), 1);
	series->append(endDate.toMSecsSinceEpoch(), 100);

	seriesPriceLevel->append(startDate.toMSecsSinceEpoch(), 100);
	seriesPriceLevel->append(endDate.toMSecsSinceEpoch(), 100);

	chartTrades->addSeries(seriesPriceLevel);
	chartTrades->addSeries(series);
	chartTrades->legend()->hide();

	axisX->setTickCount((interval / segment) + 1);
	axisX->setFormat("HH:mm::ss");
	chartTrades->addAxis(axisX, Qt::AlignBottom);
	series->attachAxis(axisX);
	seriesPriceLevel->attachAxis(axisX);

	axisY->setLabelFormat("%.2f");
	chartTrades->addAxis(axisY, Qt::AlignRight);
	series->attachAxis(axisY);
	seriesPriceLevel->attachAxis(axisY);

	series->setColor(Qt::blue);
	seriesPriceLevel->setColor(Qt::black);

	seriesPriceLevel->setPen(QPen(Qt::DashDotLine));
}

void Terminal::ChartController::update()
{
	endTime = QDateTime::currentDateTimeUtc().toTime_t();
	const auto startTime = (endTime - (endTime % segment)) - interval;

	auto tradeLoader = new Terminal::AbstractTradeLoader;
	tradeLoader->init( startTim,, endTim,, segment);
	tradeLoader->setExchange(exchangeId);
	tradeLoader->moveToThread(&threadTradesLoader);

	qRegisterMetaType<QList<TradePrice> >("QList<TradePrice>");

	connect(&threadTradesLoader, &QThread::finished, tradeLoader, &AbstractTradeLoader::stop);
	connect(&threadTradesLoader, &QThread::finished, tradeLoader, &QObject::deleteLater);
	connect(tradeLoader, &AbstractTradeLoader::sendDate, this, &ChartController::updateChart);
	connect(this, &ChartController::runLoad, tradeLoader, &AbstractTradeLoader::run);
	connect(this, &ChartController::updateSegment, tradeLoader, &AbstractTradeLoader::updateSegment);
	connect(series, &QLineSeries::pointAdded, this, &ChartController::priceLevel);

	threadTradesLoader.start();

	//emit runLoad();
	emit updateSegment();
}

QChart* Terminal::ChartController::getChartTrades()
{
	return chartTrades;
}

void Terminal::ChartController::setExchangeId(uint32_t id)
{
	exchangeId = id;
}

qreal Terminal::ChartController::max() const
{
	return axisY->max();
}

qreal Terminal::ChartController::min() const
{
	return axisY->min();
}

void Terminal::ChartController::priceLevel()
{
	auto list = series->points();

	if (list.size() <= 2) return;

	auto pointItr = list.rbegin();
	
	const auto price = pointItr->ry();

	seriesPriceLevel->clear();
	
	const auto min = axisX->min();
	const auto max = axisX->max();

	*seriesPriceLevel << QPointF(min.toMSecsSinceEpoch(), price);
	*seriesPriceLevel << QPointF(max.toMSecsSinceEpoch(), price);

	pointItr++;

	emit sendLastPrice(price, (pointItr->ry() < price ) );
}

//todo Замечена ошибка: прокрутка имеет задержку на 1 сегмент
void Terminal::ChartController::updateChart(QList<TradePrice> data)
{
	auto lastTime = endTime - (endTime % segment);

	for (auto &order : data)
	{
		const auto date = order.getDate();
		if (date <= lastTime) continue;

		series->append(date * 1000, order.getPrice());
		lastTime = date;

		auto time = QDateTime::fromTime_t(date);

		qDebug() << "New value (Date: " << time.toString("HH:mm:ss") << "; Price: " << order.getPrice();
	}
	
	if (lastTime > endTime)
	{
		auto dx = (chartTrades->plotArea().width() / interval) * (lastTime - (endTime - (endTime % segment)) );
		chartTrades->scroll(dx, 0);

		qDebug() << "Scroll to " << lastTime - endTime << "sec";

		endTime = lastTime;

		// очистка точек, которые не отображаются на графике

		auto list = series->points();

		auto max = DBL_MIN;
		auto min = DBL_MAX;

		for (auto pointItr = list.begin(); pointItr != list.end(); ++pointItr)
		{
			const auto date = pointItr->rx();

			if (date < ((endTime - interval * 2) * 1000 ))
			{
				series->remove(*pointItr);

				continue;
			}

			const auto price = pointItr->ry();

			max = std::max(max, price);
			min = std::min(min, price);
		}

		axisY->setRange(min, max);
	}
		
	priceLevel();
}
