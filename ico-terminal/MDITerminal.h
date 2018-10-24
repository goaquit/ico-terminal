#pragma once
#include "Client.h"
#include <QMainWindow>
#include <QToolBar>
#include "ui_MDITerminal.h"
#include "Options.h"
#include "ControllerOrderBook.h"
#include <QLabel>

namespace Entity
{
class Balance;
}

class MDITerminal : public QMainWindow
{
	Q_OBJECT

public:
	MDITerminal(QWidget *parent = Q_NULLPTR);

	Options * getOptions();

	void setProfileName(const QString&);
	void init();

public slots:
	void changeTimeFrame();
	void sendNotice(const QString &);
	void showStatusMessage(QString);
	void loadSettings();
	void saveSettings();
	void newChart(uint exchange, uint pair);
	void newOrderBook(const QString &title, const QString &index);
	void createChart(uint exchange, uint pair, uint timeFrame, const QUuid &);

	void addToMdi(QWidget *);

signals:
	void messageToLog(const QString &);

private:
	Ui::MDITerminal ui;

	Options options;

	Entity::Balance *balance = nullptr;

	Loader::Client clientNotice;

	QToolBar *toolBar  = nullptr;

	QString profileName = "";

	QLabel statusConnect;

	ControllerOrderBook orderBook;

	void updateMenuWindow();
	QWidget *activeGuiChart();

	void initToolBar();

	void initDockWindow();
	void initMenu();

	void initStatusBar();

	void initConnect();

private slots:
	void onActiveOrderBook(bool enable);
};
