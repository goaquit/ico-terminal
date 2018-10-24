#include "MDITerminal.h"
#include "GUIChart.h"
#include "GUIOrderBook.h"
#include "GuiOptions.h"
#include "Order.h"
#include "GuiPosition.h"
#include "GUICompare.h"
#include "GuiLog.h"
#include "GuiMartingale.h"
#include "GuiExchangePairList.h"
#include "TablePortfolio.h"

#include <QAction>

#include <QListWidgetItem>
#include <QMdiSubWindow>
#include <QDateTimeEdit>

#include <QTimer>
#include <QDateTime>
#include <QList>
#include <memory>
#include <vector>
#include <QSettings>
#include "WrapperDb.h"
#include <QVariant>
#include "Client.h"

#include <QDockWidget>
#include "Entity/Balance.h"

using namespace Terminal;
using namespace TerminalCore;
using namespace Loader;
using namespace Entity;

static const quint16 hostPortAlert = 54679;
static const quint16 portOrderBook = 54771;

static const QString configFile("./local.ini");

static const uint defaultTimeFrame = 900;

MDITerminal::MDITerminal(QWidget *parent)
	: QMainWindow(parent), balance(new Balance(parent))
{
	ui.setupUi(this);

	setWindowTitle("Terminal");

	connect(ui.menuWindow, &QMenu::aboutToShow, this, &MDITerminal::updateMenuWindow);
}


Options * MDITerminal::getOptions()
{
	return &options;
}

void MDITerminal::setProfileName(const QString &name)
{
	profileName = name;
}

void MDITerminal::init()
{
	clientNotice.start(WrapperDb::instance()->database()->hostName(), hostPortAlert);

	initMenu();
	initToolBar();

	loadSettings();
	initDockWindow();

	initStatusBar();

	initConnect();

	orderBook.start("144.76.132.240", portOrderBook);	
}

void MDITerminal::changeTimeFrame()
{
	auto window = ui.mdi->activeSubWindow()->findChild<GUIChart *>();

	if (!window) return;

	for (const auto& action : toolBar->actions())
	{
		if (action->isChecked())
		{
			const auto tf = action->data().toUInt();
			action->setChecked(false);

			window->setTimeFrame(tf);

			break;
		}
	}
}

void MDITerminal::sendNotice(const QString &msg)
{
	if (!options.getChatId() || options.isBackgroundNotice()) return;

	clientNotice.send(
		QString("id:"+QString::number(options.getChatId())+";msg:"+msg+";#end#")
	);
}

void MDITerminal::showStatusMessage(QString msg)
{
	const auto timeout = 5000;
	ui.statusBar->showMessage(msg, timeout);
}

void MDITerminal::loadSettings()
{
	QSettings setting(configFile, QSettings::IniFormat);

	auto uidList = setting.value("window/chart").toStringList();

	for (const auto &uid : uidList)
	{
		setting.beginGroup("chart-" + uid);

		const auto exchange = setting.value("exchange-id").toUInt();
		const auto pair     = setting.value("pair-id").toUInt();
		const auto tf       = setting.value("timeframe").toUInt();

		setting.endGroup();

		createChart(exchange, pair, tf, QUuid(uid));
	}

	// load options 
	setting.beginGroup("options");
		options.setChatId(setting.value("telegram-chat-id").toLongLong());
		options.setWexKey(setting.value("wex-key").toString());
		options.setWexSecret(setting.value("wex-secret").toString());
	setting.endGroup();

}

void MDITerminal::saveSettings()
{
}

void MDITerminal::newChart(uint exchange, uint pair)
{
	auto uuid = QUuid::createUuid();

	createChart(exchange, pair, defaultTimeFrame, uuid);

	// save settings for new chart
	QSettings setting(configFile, QSettings::IniFormat);

	auto chartUuuuuuuid = setting.value("window/chart").toStringList();

	chartUuuuuuuid.append(uuid.toString());

	setting.setValue("window/chart", chartUuuuuuuid);

	const auto groupName = "chart-" + uuid.toString();
	setting.beginGroup(groupName);
		setting.setValue("exchange-id", exchange);
		setting.setValue("pair-id",		pair);
		setting.setValue("timeframe",	defaultTimeFrame);
	setting.endGroup();
}

void MDITerminal::newOrderBook(const QString &title, const QString &index)
{
	auto window = new GUIOrderBook(this);

	ui.mdi->addSubWindow(window);

	window->connectControllerOrderBook(&orderBook);
	window->setWindowTitle(title);
	window->setIndex(index);
	window->show();

	updateMenuWindow();

	orderBook.requestOrderBook(index);
}

void MDITerminal::createChart(uint exchange, uint pair, uint timeFrame, const QUuid &u)
{
	auto chart = new GUIChart;
	connect(chart, &GUIChart::sendStatusMsg, this, &MDITerminal::showStatusMessage);
	chart->setTimeFrame(timeFrame);
	chart->init(exchange, pair);
	chart->setUuid(u);
	ui.mdi->addSubWindow(chart);

	chart->show();

	updateMenuWindow();

	toolBar->setEnabled(true);
}

void MDITerminal::addToMdi(QWidget * window)
{
	ui.mdi->addSubWindow(window);
}

void MDITerminal::updateMenuWindow()
{
	ui.menuWindow->clear();

	auto windowList = ui.mdi->subWindowList();
	for (const auto subWindow : windowList)
	{
		auto window = subWindow->widget();
		const auto title = window->windowTitle();

		auto action = ui.menuWindow->addAction(title, subWindow,[this, subWindow]() {
			ui.mdi->setActiveSubWindow(subWindow);
		});
		
		action->setCheckable(true);
		action->setChecked(window == activeGuiChart());
	}
}

QWidget * MDITerminal::activeGuiChart()
{
	if (QMdiSubWindow *activeSubWindow = ui.mdi->activeSubWindow())
		return activeSubWindow->widget();
	return nullptr;
}

void MDITerminal::initToolBar()
{
	struct StrTime
	{
		QString key;
		uint    time;
	};

	const uint M1 = 60;
	const uint M15 = M1 * 15;
	const uint M30 = M15 * 2;
	const uint H1 = M30 * 2;
	const uint D1 = H1 * 24;
	const uint W1 = D1 * 7;
	const uint MN = W1 * 4;

	std::vector<StrTime> timeframe;

	timeframe.push_back({ "M1",M1 });
	timeframe.push_back({ "M5",M1 * 5 });
	timeframe.push_back({ "M15",M15 });
	timeframe.push_back({ "M30",M30 });
	timeframe.push_back({ "H1",H1 });
	timeframe.push_back({ "H4",H1 * 4 });
	timeframe.push_back({ "H12",H1 * 12 });
	timeframe.push_back({ "D",D1 });


	toolBar = new QToolBar(this);
	toolBar->addWidget(new QLabel("TF:", toolBar));
	for (const auto &strTime : timeframe)
	{
		auto action = toolBar->addAction(strTime.key);
		action->setData(strTime.time);
		action->setCheckable(true);
		connect(action, &QAction::triggered, this, &MDITerminal::changeTimeFrame);
	}

	this->addToolBar(toolBar);

	toolBar->setDisabled(true);
}

void MDITerminal::initDockWindow()
{
	// Compare dock
	auto dockCompareTools = new QDockWidget("Compare", this);
	auto guiCompare = new GUICompare(this);
	dockCompareTools->setWidget(guiCompare);
	addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dockCompareTools);

	connect(guiCompare, &GUICompare::sendNotice, this, &MDITerminal::sendNotice);
	connect(guiCompare, &GUICompare::requestOrderBooks, &orderBook, &ControllerOrderBook::requestAllOrderBook);
	connect(&orderBook, &ControllerOrderBook::recievedOrderBook, guiCompare, &GUICompare::onDataRecieved);

	// portfolio dock
	auto dockPortfolio = new QDockWidget("Portfolio", this);
	auto guiPortfolio = new TablePortfolio(this);
	guiPortfolio->connectBalance(balance);
	guiPortfolio->connectControllerOrderBook(&orderBook);
	dockPortfolio->setWidget(guiPortfolio);
	addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dockPortfolio);

	// martingale dock
	auto dockMartingale = new QDockWidget("Martingale", this);
	auto guiMartingale = new GuiMartingale(this);
	guiMartingale->initConnect(balance);
	dockMartingale->setWidget(guiMartingale);
	addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dockMartingale);
	connect(&orderBook, &ControllerOrderBook::recievedOrderBook, [guiMartingale](const OrderBook &orderBook, const QString &index)
	{
		// select ask price
		if (orderBook.asks().empty()) return;
		const auto ask = orderBook.asks().begin()->getPrice();

		guiMartingale->onChangedAsk(ask, index);

		// select bid price
		if (orderBook.bids().empty()) return;
		const auto bid = orderBook.bids().begin()->getPrice();

		guiMartingale->onChangedBid(bid, index);
	});
	connect(guiMartingale, &GuiMartingale::reuqestAsk, &orderBook, &ControllerOrderBook::requestOrderBook);

	// exchange pair list
	auto guiExchangePairList = new GuiExchangePairList(this);
	guiExchangePairList->connectOrderBook(&orderBook);	
	guiExchangePairList->init();
	connect(guiExchangePairList, &GuiExchangePairList::newChart, this, &MDITerminal::newChart);
	connect(guiExchangePairList, &GuiExchangePairList::newOrderBook, this, &MDITerminal::newOrderBook);

	auto dockExchangePairList = new QDockWidget("Exchange list", this);
	dockExchangePairList->setWidget(guiExchangePairList);
	addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dockExchangePairList);

	// init
	ui.menuView->addAction(dockCompareTools->toggleViewAction());
	ui.menuView->addAction(dockPortfolio->toggleViewAction());
	ui.menuView->addAction(dockMartingale->toggleViewAction());
	ui.menuView->addAction(dockExchangePairList->toggleViewAction());

	tabifyDockWidget(dockCompareTools, dockPortfolio);
	tabifyDockWidget(dockPortfolio, dockMartingale);

	dockCompareTools->close();
	dockExchangePairList->close();

	orderBook.requestAllOrderBook();
}

void MDITerminal::initMenu()
{
	ui.menuCharts->clear();

	auto query = WrapperDb::instance()->select("SELECT id, name FROM exchange ORDER BY name ASC;");

	while (query->next())
	{
		const auto exchange = query->value(1).toString().trimmed();
		const auto id = query->value(0).toUInt();

		auto queryPair = WrapperDb::instance()->select("select id, title from getpairsstrexchange(" + QString::number(id) + ") order by title;");

		if (queryPair->lastError().isValid()) continue; 

		auto menu = ui.menuCharts->addMenu(exchange);
		while (queryPair->next())
		{
			const auto pairName = queryPair->value(1).toString().trimmed();
			const auto pairId = queryPair->value(0).toUInt();

			auto action = menu->addAction(pairName);

			QObject::connect(action, &QAction::triggered, [this, id, pairId]()
			{
				auto uuid = QUuid::createUuid();

				createChart(id, pairId, defaultTimeFrame, uuid);

				//todo: fix duplication in createWindowChart method
				// save settings for new chart
				QSettings setting(configFile, QSettings::IniFormat);

				auto chartUuuuuuuid = setting.value("window/chart").toStringList();

				chartUuuuuuuid.append(uuid.toString());

				setting.setValue("window/chart", chartUuuuuuuid);

				const auto groupName = "chart-" + uuid.toString();
				setting.beginGroup(groupName);
				setting.setValue("exchange-id", id);
				setting.setValue("pair-id", pairId);
				setting.setValue("timeframe", defaultTimeFrame);
				setting.endGroup();
			});
		}

		ui.menuCharts->addMenu(menu);
	}


	ui.menuTools->clear();

	auto action = ui.menuTools->addAction("Options");
	QObject::connect(action, &QAction::triggered, [this]()
	{
		auto optionWindow = new GuiOptions;
		connect(this, &QObject::destroyed, optionWindow, &QObject::deleteLater);
		optionWindow->setOption(&options);
		optionWindow->show();
	});
}

void MDITerminal::initStatusBar()
{
	statusConnect.setMaximumWidth(150); statusConnect.setMinimumWidth(150); statusConnect.setText("Status server: Offline");
	ui.statusBar->insertPermanentWidget(0, &statusConnect);
}

void MDITerminal::initConnect()
{
	connect(&orderBook, &ControllerOrderBook::isActive, this, &MDITerminal::onActiveOrderBook);
}

void MDITerminal::onActiveOrderBook(bool enable)
{
	QString msg = "Status server: " + QString((enable) ? "Online" : "Offline");
	statusConnect.setText( msg );

	// after success connect to server connecting signal/slots Balance
	balance->synchronize();
}
