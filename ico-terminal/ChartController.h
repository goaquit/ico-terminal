#ifndef TERMINAL_CHART_CONTROLLER_H
#define TERMINAL_CHART_CONTROLLER_H

#include "TradePrice.h"

#include <ctime>

#include <QObject>
#include <QThread>
#include <QList>

#include <QtCharts\QChart>
#include <QtCharts\QLineSeries>
#include <QtCharts\QDateTimeAxis>
#include <QtCharts\QValueAxis>

using namespace QtCharts;

namespace Terminal
{
	class ChartController : public QObject
	{
		Q_OBJECT

		QThread threadTradesLoader;

	public:
		ChartController(QObject *parent);
		~ChartController();

		void run();

		QChart* getChartTrades();
		void setExchangeId(uint32_t);

		qreal max() const;
		qreal min() const;
		
	public slots:
		void updateChart(QList<TradePrice>);

	private:
		QChart *chartTrades;

		QGraphicsSimpleTextItem *text;

		QLineSeries   *seriesPriceLevel;
		QLineSeries   *series;
		QDateTimeAxis *axisX;
		QValueAxis    *axisY;

		time_t interval = 60 * 1;
		time_t segment  = 5;
		time_t endTime;

		uint32_t exchangeId;

		void priceLevel();
		void firstLoad();
		void update();

	signals:
		void runLoad();
		void updateSegment();
		void sendLastPrice(double, bool);
	};
}

#endif TERMINAL_CHART_CONTROLLER_H