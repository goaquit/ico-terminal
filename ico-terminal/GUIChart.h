#pragma once

#include "ui_GUIChart.h"
#include "TradePrice.h"

#include <QMainWindow>
#include <QToolBar>

#include <QTimer>
#include <QThread>
#include <QUuid>
#include <QSqlDataBase>

#include <ctime>
#include <vector>

class QGraphicsSimpleTextItem;

namespace DataStorage
{
class TradeStorage;
}

namespace ChartCore
{
class Chart;
class Series;
}

namespace Indicators
{
class AbstractIndicator;
}

using namespace Entity;
using namespace Terminal;

class GUIChart : public QMainWindow
{
	Q_OBJECT

public:
	GUIChart(QWidget *parent = Q_NULLPTR);
	~GUIChart();
	
	void init(uint32_t indexExchange, uint32_t indexPair);
	void clearAllSeries();

	void setTimeFrame(uint);

	      void  setUuid(const QUuid &);
	const QUuid getUuid() const;

public slots:
	void onDataReceive(const QList<TradePrice> &);
	void onLastDataReplace(const TradePrice &);
	// parameter in config file
	void writeIndicator(const Indicators::AbstractIndicator *);
	void readIndicators();
	void removeIndicator(Indicators::AbstractIndicator *);

signals:
	void sendStatusMsg(QString);
	void stopAllThread();
	void scrollToLastDate();
	void changeRangeMain(qreal max, qreal min);
	void changeRangeAmount(qreal max);
	void changeSegment(int);
	void changeTimeFrame(uint);
	void closeIndicatorList();
	void drawCharts();

private:
	Ui::GUIChartClass ui;
	QUuid uuid;

	QString title;

	void GUIChart::readSetting();

	// data
	DataStorage::TradeStorage *dataStorage = nullptr;
	QThread threadDataStorage;
	void startDataStorage();

	uint timeframe = 900;
	uint timeFrameCount = 120;

	// charts
	void initSeries();

	// series
	ChartCore::Series *seriesMain;
	std::vector<Indicators::AbstractIndicator *> indicators;

	// action
	void createActions();

	QAction *indicatorsListAct;
	QAction *addAdxAct;
	QAction *addBollingerAct;
	QAction *addSmaAct;
	QAction *addParabolicSarAct;
	QAction *addStochasticAct;

protected:
	void GUIChart::closeEvent(QCloseEvent *event);
	void GUIChart::resizeEvent(QResizeEvent * event);
	void GUIChart::contextMenuEvent(QContextMenuEvent * event);

private slots:
	void onShowListIndicators();
	void onAddAdx();
	void onAddBollinger();
	void onAddSar();
	void onAddSma();
	void onAddStochastic();
};