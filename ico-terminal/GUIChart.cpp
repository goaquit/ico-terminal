#include "GUIChart.h"
#include "GuiIndicatorsList.h"
#include "AbstractTradeLoader.h"
#include "Chart.h"
#include "IndicatorBollinger.h"
#include "IndicatorDx.h"
#include "IndicatorParabollicSar.h"
#include "IndicatorSma.h"
#include "IndicatorStochastic.h"
#include "WrapperDb.h"
#include "Series.h"
#include "TradeStorage.h"


// Qt lib include
#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QMenu>
#include <QSqlQuery>
#include <QSqlError>

#include <QtCharts\QLineSeries>
#include <QtCharts\QDateTimeAxis>
#include <QtCharts\QValueAxis>

#include <QtWidgets/QGraphicsTextItem>

#include <QDateTime>
#include <QSettings>
#include <QTableWidgetItem>
#include <QTableWidget>
///////////////////////////////////


using namespace QtCharts;
using namespace ChartCore;
using namespace DataStorage;
using namespace TerminalCore;
using namespace Entity;

const QString configFile         = "./local.ini";
const uint    maxAdditionalChart = 5;

QString GetWindowTitle(uint exchange, uint pair)
{
	QString title;

	auto query = WrapperDb::instance()->select("select name from exchange where id = " + QString::number(exchange) + " limit 1;");

	if (query->next())
	{
		title = query->value("name").toString();
	}

	delete query;

	query = WrapperDb::instance()->select("select c1.name as first, c2.name as second from pair \
		left join currency as c1 on c1.id = pair.first \
		left join currency as c2 on c2.id = pair.second \
		where pair.id = '" + QString::number(pair) + "'; ");

	if (query->lastError().isValid())
	{
		qDebug() << query->lastError().databaseText();
		return "";
	}

	if (query->next())
	{
		title += QString("::") + query->value("first").toString() + query->value("second").toString();
	}

	delete query;

	return title;
}


//////////////////////////////////////////////
// write indicator parameter to config file // //todo: go hahui from this file
//////////////////////////////////////////////
void writeIndicatorParameter(const Indicators::IndicatorDx *indicator, QSettings &setting)
{
	setting.setValue("period", indicator->getPeriod());
	
	setting.beginGroup("adx");
	setting.setValue("color", indicator->getAdxColor());
	setting.setValue("size", indicator->getAdxSize());
	setting.endGroup();

	setting.beginGroup("+dm");
	setting.setValue("color", indicator->getPdiColor());
	setting.setValue("size", indicator->getPdiSize());
	setting.endGroup();

	setting.beginGroup("-dm");
	setting.setValue("color", indicator->getNdiColor());
	setting.setValue("size", indicator->getNdiSize());
	setting.endGroup();
}

void writeIndicatorParameter(const Indicators::IndicatorBollinger *indicator, QSettings &setting)
{
	setting.setValue("period", indicator->getPeriod());
	setting.setValue("multiplier", indicator->getMultiplier());
	setting.beginGroup("series");
	setting.setValue("color", indicator->getColor());
	setting.setValue("size", indicator->getSize());
	setting.endGroup();
}

void writeIndicatorParameter(const Indicators::IndicatorParabollicSar *indicator, QSettings &setting)
{
	setting.setValue("max", indicator->getMax());
	setting.setValue("step", indicator->getStep());

	setting.beginGroup("series");
	setting.setValue("color", indicator->getColor());
	setting.setValue("size", indicator->getSize());
	setting.endGroup();
}

void writeIndicatorParameter(const Indicators::IndicatorSma *indicator, QSettings &setting)
{
	setting.setValue("period", indicator->getPeriod());

	setting.beginGroup("series");
	setting.setValue("color", indicator->getColor());
	setting.setValue("size", indicator->getSize());
	setting.endGroup();
}

void writeIndicatorParameter(const Indicators::IndicatorStochastic *indicator, QSettings &setting)
{
	setting.setValue("fast_period", indicator->getFastPeriod());
	setting.setValue("slow_period", indicator->getSlowPeriod());
	setting.setValue("smooth_period", indicator->getSmooth());

	setting.beginGroup("fast");
	setting.setValue("color", indicator->getFastColor());
	setting.setValue("size", indicator->getFastSize());
	setting.endGroup();

	setting.beginGroup("slow");
	setting.setValue("color", indicator->getSlowColor());
	setting.setValue("size", indicator->getSlowSize());
	setting.endGroup();
}

///////////////////////////////////////////////
// read indicator parameter from config file //
///////////////////////////////////////////////

Indicators::IndicatorDx * restoreIndicatorAdx(QSettings &setting, Terminal::ChartView *view)
{
	if (view->getAdditionalChartCount() >= maxAdditionalChart) return new Indicators::IndicatorDx;

	auto chart = new ChartCore::Chart;
	view->addAdditionalChart(chart);
	chart->setRangeVertical(100, 0);
	chart->setVerticalTick(4);
	chart->setInfo("ADX");

	auto indicator = new Indicators::IndicatorDx;
	Indicators::AbstractIndicator::connect(indicator, &Indicators::AbstractIndicator::destroyed,
		[view, chart]()
	{
		view->onRemoveChart(chart);
		view->drawCharts();
	});

	indicator->setChart(chart);

	
	indicator->setPeriod(setting.value("period").toInt());

	setting.beginGroup("adx");
		indicator->changeAdxColor(setting.value("color").value<QColor>());
		indicator->changeAdxSize(setting.value("size").toInt());
	setting.endGroup();

	setting.beginGroup("+dm");
		indicator->changePdiColor(setting.value("color").value<QColor>());
		indicator->changePdiSize(setting.value("size").toInt());
	setting.endGroup();

	setting.beginGroup("-dm");
		indicator->changeNdiColor(setting.value("color").value<QColor>());
		indicator->changeNdiSize(setting.value("size").toInt());
	setting.endGroup();

	return indicator;
}

Indicators::IndicatorBollinger * restoreIndicatorBollinger(QSettings &setting, Terminal::ChartView *view)
{
	auto indicator = new Indicators::IndicatorBollinger;

	view->addToMain(indicator->getTopLine());
	view->addToMain(indicator->getBottomLine());

	indicator->setPeriod(setting.value("period").toInt());
	indicator->setMultiplier(setting.value("multiplier").toUInt());

	setting.beginGroup("series");
	indicator->changeColor(setting.value("color").value<QColor>());
	indicator->changeSize(setting.value("size").toInt());
	setting.endGroup();

	return indicator;
}

Indicators::IndicatorParabollicSar * restoreIndicatorSar(QSettings &setting, Terminal::ChartView *view)
{
	auto indicator = new Indicators::IndicatorParabollicSar;

	view->addToMain(indicator->getSeries());

	indicator->setMax(setting.value("max").toDouble());
	indicator->setStep(setting.value("step").toDouble());

	setting.beginGroup("series");
		indicator->changeColor(setting.value("color").value<QColor>());
		indicator->changeSize(setting.value("size").toDouble());
	setting.endGroup();

	return indicator;
}

Indicators::IndicatorSma * restoreIndicatorSma(QSettings &setting, Terminal::ChartView *view)
{
	auto indicator = new Indicators::IndicatorSma;

	view->addToMain(indicator->getSeriesSma());

	indicator->setPeriod(setting.value("period").toInt());

	setting.beginGroup("series");
		indicator->changeColor(setting.value("color").value<QColor>());
		indicator->changeSize(setting.value("size").toInt());
	setting.endGroup();

	return indicator;
}

Indicators::IndicatorStochastic *restoreIndicatorStochastic(QSettings &setting, Terminal::ChartView *view)
{
	auto indicator = new Indicators::IndicatorStochastic;

	if (view->getAdditionalChartCount() >= maxAdditionalChart) return indicator;

	auto chart = new ChartCore::Chart;
	Indicators::AbstractIndicator::connect(indicator, &Indicators::AbstractIndicator::destroyed, 
		[view, chart]()
	{
		view->onRemoveChart(chart);
		view->drawCharts();
	});

	view->addAdditionalChart(chart);
	chart->setRangeVertical(100, 0);

	std::vector<double> ticks = { 20.0,80.0 };
	chart->setVerticalTickValue(ticks);
	chart->setInfo("Stochastic");

	indicator->setChart(chart);

	indicator->setFastPeriod(setting.value("fast_period").toUInt());
	indicator->setSlowPeriod(setting.value("slow_period").toUInt());
	indicator->setSmooth(setting.value("smooth_period").toUInt());

	setting.beginGroup("fast");
	indicator->changeFastColor(setting.value("color").value<QColor>());
	indicator->changeFastSize(setting.value("size").toInt());
	setting.endGroup();

	setting.beginGroup("slow");
	indicator->changeSlowColor(setting.value("color").value<QColor>());
	indicator->changeSlowSize(setting.value("size").toInt());
	setting.endGroup();

	return indicator;
}

///////////////////////////////////////////////
GUIChart::GUIChart(QWidget *parent)
	: QMainWindow(parent), dataStorage(new DataStorage::TradeStorage), seriesMain(new Series)
{
	ui.setupUi(this);

	createActions();

	connect(this, &GUIChart::drawCharts, ui.chartView, &ChartView::drawCharts);
}

GUIChart::~GUIChart()
{
	emit stopAllThread();
		
	threadDataStorage.quit();
	threadDataStorage.wait();

	delete seriesMain;
}

void GUIChart::setTimeFrame(uint tf)
{
	if (!tf)
	{
		tf = 900;
	}

	timeframe = tf;
	clearAllSeries();
	dataStorage->setSegment(timeframe * timeFrameCount);
	dataStorage->setTimeFrame(timeframe);
	ui.chartView->onMainChartInfo(dataStorage->getTimeframeText());

	setWindowTitle(title + ":" + dataStorage->getTimeframeText());

	emit changeTimeFrame(tf);

	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("chart-" + uuid.toString());
		setting.setValue("timeframe", tf);
	setting.endGroup();
}

void GUIChart::setUuid(const QUuid &u)
{
	if (uuid.isNull())
	{
		uuid = u;
	}

	readSetting();
}

const QUuid GUIChart::getUuid() const
{
	return uuid;
}

void GUIChart::onDataReceive(const QList<TradePrice> &list)
{
	for (const auto trade : list)
	{
		seriesMain->append(trade);
	}

	const auto lastTrade = *list.rbegin();

	ui.chartView->setLastTradePrice(lastTrade);

	const auto segment = dataStorage->getSegment();
	seriesMain->processRange(lastTrade.getDate() - segment, lastTrade.getDate());

	emit changeSegment(segment);
	emit scrollToLastDate();
	emit changeRangeMain(seriesMain->getMax(), seriesMain->getMin());
	emit changeRangeAmount(seriesMain->getMaxAmount());
}

void GUIChart::onLastDataReplace(const TradePrice &trade)
{
	ui.chartView->setLastTradePrice(trade);

	seriesMain->removeLast();
	seriesMain->append(trade);

	seriesMain->processRange(trade.getDate() - dataStorage->getSegment(), trade.getDate());
	emit changeRangeMain(seriesMain->getMax(), seriesMain->getMin());
	emit changeRangeAmount(seriesMain->getMaxAmount());
}

void GUIChart::writeIndicator(const Indicators::AbstractIndicator *indicator)
{
	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("chart-" + uuid.toString());

	const auto uidStr = indicator->getUuid().toString();
	auto uidList = setting.value("indicators").toStringList();
	if (!uidList.contains(uidStr))
	{
		uidList << uidStr;
		setting.setValue("indicators", uidList);
	}

	setting.beginGroup("indicator-" + uidStr);

	setting.setValue("type", indicator->getType());
	setting.setValue("alias", indicator->getAlias());

	switch (indicator->getType())
	{
	case Indicators::AbstractIndicator::Adx :
		writeIndicatorParameter(static_cast<const Indicators::IndicatorDx *>(indicator), setting);
			break;	
	case Indicators::AbstractIndicator::Bollinger:
		writeIndicatorParameter(static_cast<const Indicators::IndicatorBollinger *>(indicator), setting);
		break;
	case Indicators::AbstractIndicator::Sar: 
		writeIndicatorParameter(static_cast<const Indicators::IndicatorParabollicSar *>(indicator), setting);
		break;
	case Indicators::AbstractIndicator::Sma: 
		writeIndicatorParameter(static_cast<const Indicators::IndicatorSma *>(indicator), setting);
		break;
	case Indicators::AbstractIndicator::Stochastic:
		writeIndicatorParameter(static_cast<const Indicators::IndicatorStochastic *>(indicator), setting);
		break;
	}

	setting.endGroup();
	setting.endGroup();
}

void GUIChart::readIndicators()
{
	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("chart-" + uuid.toString());
	const auto uidList = setting.value("indicators").toStringList();

	for (const auto uidStr : uidList)
	{
		Indicators::AbstractIndicator *indicator = nullptr;
		
		setting.beginGroup("indicator-" + uidStr);

		auto type = setting.value("type").toInt();

		switch (type)
		{
		case  Indicators::AbstractIndicator::Adx: 
			indicator = restoreIndicatorAdx(setting, ui.chartView);
			break;
		case  Indicators::AbstractIndicator::Bollinger:
			indicator = restoreIndicatorBollinger(setting, ui.chartView);
			break;
		case  Indicators::AbstractIndicator::Sar:
			indicator = restoreIndicatorSar(setting, ui.chartView);
			break;
		case  Indicators::AbstractIndicator::Sma:
			indicator = restoreIndicatorSma(setting, ui.chartView);
			break;
		case  Indicators::AbstractIndicator::Stochastic:
			indicator = restoreIndicatorStochastic(setting, ui.chartView);
			break;
		}

		if (indicator)
		{
			indicator->setAlias(setting.value("alias").toString());
			indicator->setUuid(QUuid(uidStr));
			indicators.push_back(indicator);
			dataStorage->connectIndicator(indicator);

			//emit indicator->requestData();
		}

		setting.endGroup();
	}

	setting.endGroup();

	if (indicators.size())
	{
		emit (*indicators.begin())->requestData();
	}

	emit drawCharts();
}

void GUIChart::removeIndicator(Indicators::AbstractIndicator *indicator)
{
	for (auto iter = indicators.begin(); iter != indicators.end(); )
		(*iter == indicator) ? iter = indicators.erase(iter) : ++iter;

	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("chart-" + uuid.toString());


	auto uidList = setting.value("indicators").toStringList();
	uidList.removeOne(indicator->getUuid().toString());

	setting.setValue("indicators", uidList);
	setting.remove("indicator-" + indicator->getUuid().toString());

	setting.endGroup();

	indicator->deleteLater();
}

void GUIChart::onShowListIndicators()
{
	emit closeIndicatorList();

	auto window = new Gui::GuiIndicatorsList;

	auto conn = connect(this, &GUIChart::closeIndicatorList, [window]() {
		window->close(); 
	});

	connect(window, &Gui::GuiIndicatorsList::destroyed, [this, conn]()
	{
		this->disconnect(conn);
	});

	connect(window, &Gui::GuiIndicatorsList::writeIndicator, this, &GUIChart::writeIndicator);
	connect(window, &Gui::GuiIndicatorsList::removeIndicator, this, &GUIChart::removeIndicator);

	window->setIndicators(indicators);
	window->show();
}

void GUIChart::init(uint32_t exch, uint32_t p)
{
	title = GetWindowTitle(exch, p);
	setWindowTitle(title + ":" + dataStorage->getTimeframeText());

	connect(this, &GUIChart::scrollToLastDate,  ui.chartView, &ChartView::scrollToLastDate);
	connect(this, &GUIChart::changeRangeMain,   ui.chartView, &ChartView::changeRangeMain);
	connect(this, &GUIChart::changeRangeAmount, ui.chartView, &ChartView::changeRangeAmount);
	connect(this, &GUIChart::changeSegment,     ui.chartView, &ChartView::changeSegment);
	
	connect(ui.chartView, &ChartView::requestDateInfo,             dataStorage,  &DataStorage::TradeStorage::onDateInfo);
	connect(dataStorage,  &DataStorage::TradeStorage::sendDateInfo, ui.chartView, &ChartView::onMainChartInfo);


	dataStorage->setExchange(exch);
	dataStorage->setPair(p);

	startDataStorage();

	initSeries();


	emit sendStatusMsg("Please wait. Chart initialization...");
}

void GUIChart::clearAllSeries()
{
	seriesMain->clear();
	for (auto series : indicators)
	{
		series->clear();
	}
}

void GUIChart::readSetting()
{
	if (uuid.isNull()) return;

	QSettings setting(configFile, QSettings::IniFormat);
	
	setting.beginGroup("chart-" + uuid.toString());
	const auto r = setting.value("rect").toRect();


	setting.endGroup();

	readIndicators();
}

void GUIChart::startDataStorage()
{
	dataStorage->moveToThread(&threadDataStorage);
	//dataStorage->setTimeFrame(timeframe);
	//dataStorage->setSegment(timeframe * timeFrameCount);

	//connect
	qRegisterMetaType<QList<TradePrice> >("QList<TradePrice>");
	qRegisterMetaType<TradePrice>("TradePrice");

	connect(&threadDataStorage, &QThread::started, dataStorage, &TradeStorage::run);
	connect(&threadDataStorage, &QThread::finished, dataStorage, &TradeStorage::deleteLater);
	connect(this, &GUIChart::stopAllThread, dataStorage, &TradeStorage::finish);
	connect(dataStorage, &TradeStorage::sendNewData, this, &GUIChart::onDataReceive);
	connect(dataStorage, &TradeStorage::changedLastTrade, this, &GUIChart::onLastDataReplace);
	connect(this, &GUIChart::changeTimeFrame, dataStorage, &TradeStorage::changeTimeFrame);

	threadDataStorage.start();
}

void GUIChart::initSeries()
{
	seriesMain->init(ChartSeriesType::Candlestick);
	ui.chartView->setMainSeries(seriesMain->getSeries());
	ui.chartView->setAmountSeries(seriesMain->getAmount());
	
	const auto endTime = QDateTime::currentDateTime().toTime_t();
	const auto endDate = QDateTime::fromTime_t(endTime - endTime % timeframe);
	const qint64 offset = dataStorage->getSegment();
	const auto negativeOffset = -offset;
	const auto startDate = endDate.addSecs(negativeOffset);

	ui.chartView->setMaxDate(endDate);
	ui.chartView->setMinDate(startDate);

	emit changeRangeMain(1, 0);
	emit changeRangeAmount(1);
}

void GUIChart::createActions()
{
	indicatorsListAct = new QAction("Indicators list", this);
	connect(indicatorsListAct, &QAction::triggered, this, &GUIChart::onShowListIndicators);

	addAdxAct          = new QAction("Average Directional Index (ADX)", this);
	addBollingerAct    = new QAction("Bollinger Bands", this);
	addParabolicSarAct = new QAction("Parabolic SAR", this);
	addSmaAct          = new QAction("Simple Moving Average (SMA)", this);
	addStochasticAct   = new QAction("Stochastic Oscillator", this);

	connect(addAdxAct, &QAction::triggered, this, &GUIChart::onAddAdx);
	connect(addBollingerAct, &QAction::triggered, this, &GUIChart::onAddBollinger);
	connect(addParabolicSarAct, &QAction::triggered, this, &GUIChart::onAddSar);
	connect(addSmaAct, &QAction::triggered, this, &GUIChart::onAddSma);
	connect(addStochasticAct, &QAction::triggered, this, &GUIChart::onAddStochastic);
	
}

void GUIChart::closeEvent(QCloseEvent * event)
{
	QSettings setting(configFile,QSettings::IniFormat);

	auto uidList = setting.value("window/chart").toStringList();
	uidList.removeOne(uuid.toString());

	setting.setValue("window/chart", uidList);

	setting.remove("chart-" + uuid.toString());

	QMainWindow::closeEvent(event);
}

void GUIChart::resizeEvent(QResizeEvent * event)
{
	QSettings setting(configFile, QSettings::IniFormat);

	setting.beginGroup("chart-" + uuid.toString());


	setting.endGroup();

	QMainWindow::resizeEvent(event);
}

void GUIChart::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu menu(this);
	auto indicatorAdd = menu.addMenu("Add indicator");
	auto oscillatorAdd = menu.addMenu("Add oscillator");
	menu.addSeparator();
	menu.addAction(indicatorsListAct);

	indicatorAdd->addAction(addAdxAct);
	indicatorAdd->addAction(addBollingerAct);
	indicatorAdd->addAction(addSmaAct);
	indicatorAdd->addAction(addParabolicSarAct);

	oscillatorAdd->addAction(addStochasticAct);

	menu.exec(event->globalPos());
}

void GUIChart::onAddAdx()
{
	if (ui.chartView->getAdditionalChartCount() >= maxAdditionalChart) return;

	const uint defaulPeriod = 14;

	auto chart = new ChartCore::Chart;
	ui.chartView->addAdditionalChart(chart);

	chart->setRangeVertical(100, 0);
	chart->setVerticalTick(4);
	chart->setInfo("ADX");

	auto indicator = new Indicators::IndicatorDx;
	connect(indicator, &Indicators::AbstractIndicator::destroyed, [this, chart]()
	{
		ui.chartView->onRemoveChart(chart);
		ui.chartView->drawCharts();
	});

	indicator->setUuid(QUuid::createUuid());
	indicator->setAlias("ADX");
	indicators.push_back(indicator);
	indicator->setChart(chart);
	indicator->setPeriod(defaulPeriod);
	dataStorage->connectIndicator(indicator);

	writeIndicator(indicator);

	emit indicator->requestData();
	emit drawCharts();
}

void GUIChart::onAddBollinger()
{
	auto indicator = new Indicators::IndicatorBollinger;
	indicator->setUuid(QUuid::createUuid());
	indicators.push_back(indicator);
	ui.chartView->addToMain(indicator->getTopLine());
	ui.chartView->addToMain(indicator->getBottomLine());

	indicator->changeColor(indicator->getColor());// apply 1 color to 2 series

	dataStorage->connectIndicator(indicator);

	writeIndicator(indicator);

	emit indicator->requestData();
}

void GUIChart::onAddSar()
{
	auto indicator = new Indicators::IndicatorParabollicSar;
	indicator->setUuid(QUuid::createUuid());
	indicator->setAlias("Parabollic SAR");
	indicators.push_back(indicator);
	ui.chartView->addToMain(indicator->getSeries());

	dataStorage->connectIndicator(indicator);

	writeIndicator(indicator);

	emit indicator->requestData();
}

void GUIChart::onAddSma()
{
	const uint defaultPeriod = 14;

	auto indicator = new Indicators::IndicatorSma;
	indicator->setUuid(QUuid::createUuid());
	indicator->setAlias("SMA");
	indicators.push_back(indicator);
	ui.chartView->addToMain(indicator->getSeriesSma());
	indicator->setPeriod(defaultPeriod);
	dataStorage->connectIndicator(indicator);

	writeIndicator(indicator);

	emit indicator->requestData();
}

void GUIChart::onAddStochastic()
{
	if (ui.chartView->getAdditionalChartCount() >= maxAdditionalChart) return;

	const uint defaultFastPeriod = 14;
	const uint defaultSlowPeriod = 3;

	auto chart = new ChartCore::Chart;
	ui.chartView->addAdditionalChart(chart);
	chart->setRangeVertical(100, 0);

	std::vector<double> ticks = { 20.0,80.0 };
	chart->setVerticalTickValue(ticks);
	chart->setInfo("Stochastic");

	auto indicator = new Indicators::IndicatorStochastic;
	connect(indicator, &Indicators::AbstractIndicator::destroyed, [this,chart]()
	{
		ui.chartView->onRemoveChart(chart);
		ui.chartView->drawCharts();
	});

	indicator->setUuid(QUuid::createUuid());
	indicator->setAlias("Stochastic");
	indicators.push_back(indicator);
	indicator->setChart(chart);
	indicator->setFastPeriod(defaultFastPeriod);
	indicator->setSlowPeriod(defaultSlowPeriod);
	dataStorage->connectIndicator(indicator);

	writeIndicator(indicator);

	emit indicator->requestData();
	emit drawCharts();
}
