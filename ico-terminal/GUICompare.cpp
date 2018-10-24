#include "GUICompare.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QItemEditorFactory>
#include <QPair>
#include "WrapperDb.h"

#include "ColorListEditor.h"
#include "TableItem.h"
#include <QTimerEvent>

#include <vector>

using namespace Entity;
using namespace TerminalCore;

static const quint16 hostPort                    = 54769;
       const QString settingFile                 = "./local.ini";
       const QString propertyGroup               = "compare-exchange";
       const QString propertyKeySelectedPair     = "selected-id-pair";
       const QString propertyKeySelectedExchange = "selected-id-exchange";
       const QString propertyKeyActive           = "active";
       const QString propertyKeyPercent          = "percent";
       const QString propertyKeyVolume           = "volume";

static const uint printTimerInterval = 1000;

GUICompare::GUICompare(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle("Exchange compare tool");	

	initCurrnecyPair();
	initRows();
	loadProperty();
	initTable();

	//connect(&timerPrint,          &QTimer::timeout,           this, &GUICompare::print);
	connect(ui.tableCurrencyPair, &QTableWidget::itemChanged, this, &GUICompare::onCurrencyPairSelected);
	connect(ui.tableExchange,     &QTableWidget::itemChanged, this, &GUICompare::onExchangeSelected);
	connect(ui.selectAllExchange, &QCheckBox::stateChanged,   this, &GUICompare::onAllSelectedExchange);
	connect(ui.selectAllPairs,    &QCheckBox::stateChanged,   this, &GUICompare::onAllSelectedPair);

	// filter connect
	connect(ui.filter, &QCheckBox::stateChanged, [this](int) { print(); saveProperty(); });
	connect(ui.levelPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d) {Q_UNUSED(d); print(); saveProperty();  });
	connect(ui.levelVolume,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double d) {Q_UNUSED(d); print(); saveProperty();  });


	// notification connect
	connect(ui.notification, &QCheckBox::stateChanged, [this](int) { saveProperty(); });
	connect(ui.message, &QTextEdit::textChanged, [this]() { saveProperty(); });

	// color indicator
	connect(ui.addIndicator,    &QPushButton::released, this, &GUICompare::onAddColorIndicator);
	connect(ui.removeIndicator, &QPushButton::released, this, &GUICompare::onRemoveColorIndicator);

	//====
	connect(ui.colorIndicator, &QTableWidget::itemChanged, [this](QTableWidgetItem * item) {Q_UNUSED(item); print(); saveProperty();  });

	//client.start(WrapperDb::instance()->database()->hostName(), hostPort);

	QItemEditorFactory *factory = new QItemEditorFactory;

	auto colorListCreator = new QStandardItemEditorCreator<ColorListEditor>();
	auto spinBox = new QStandardItemEditorCreator<QSpinBox>();
	//auto line = new QStandardItemEditorCreator<QLineEdit>();
	auto doubleSpin = new QStandardItemEditorCreator<QDoubleSpinBox>();

	factory->registerEditor(QVariant::Color, colorListCreator);
	factory->registerEditor(QVariant::Int, spinBox);
	factory->registerEditor(QVariant::Double, doubleSpin);

	QItemEditorFactory::setDefaultFactory(factory);

	startTimer(printTimerInterval);
}

void GUICompare::onDataRecieved(const OrderBook & orderBook, const QString & index)
{
	for (auto &row : rows)
	{
		QString indexBuy  = (row.exchangeBuy.second  + "_" + row.pair.second).toLower();
		QString indexSell = (row.exchangeSell.second + "_" + row.pair.second).toLower();

		if (!index.compare(indexBuy))
		{
			if (orderBook.asks().empty()) continue;

			row.ask = *orderBook.asks().begin();
		}
		else if (!index.compare(indexSell))
		{
			if (orderBook.bids().empty()) continue;
			row.bid = *orderBook.bids().begin();
		}
	}
}

void GUICompare::timerEvent(QTimerEvent *)
{
	// timer print table event
	print();
}

void GUICompare::initCurrnecyPair()
{
	currencyPair.clear();
	ui.tableCurrencyPair->clearContents();

	auto query = WrapperDb::instance()->select("select id,title from get_available_pair();");
	//TODO: check query error

	ui.tableCurrencyPair->setRowCount(query->numRowsAffected());

	auto rowCounter = 0;
	while (query->next())
	{
		PairInfo pair(query->value(0).toUInt(), query->value(1).toString());
		currencyPair.push_back(pair);

		auto item = new QTableWidgetItem(pair.second);
		item->setFlags(Qt::ItemIsUserCheckable  | Qt::ItemIsEnabled);
		
		const auto checkState = (std::find(selectedPairs.begin(), selectedPairs.end(), pair.first) != selectedPairs.end()) ?
			Qt::Checked : Qt::Unchecked;
		item->setCheckState(checkState);
		item->setData(Qt::UserRole + QVariant::UInt, pair.first);
		ui.tableCurrencyPair->setItem(rowCounter, 0, item);

		++rowCounter;
	}
}

void GUICompare::initRows()
{
	rows.clear();

	auto query = WrapperDb::instance()->select("select distinct e.id, e.name \
		from trades_last_index as t\
		left join exchange as e on e.id = t.exchange\
		order by name;");

	//TODO: check query error
	ui.tableExchange->clearContents();
	ui.tableExchange->setRowCount(query->numRowsAffected());
	uint rowCounter = 0;
	while (query->next())
	{
		ExchangeInfo exchangeInfo{ query->value(0).toUInt(),query->value(1).toString().replace(".", "") };
		exchangePairs.push_back({ exchangeInfo });

		// init table exchenge select
		auto item = new QTableWidgetItem(exchangeInfo.second);
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setData(Qt::UserRole + QVariant::UInt, exchangeInfo.first);
		ui.tableExchange->setItem(rowCounter, 0, item);

		++rowCounter;
	}

	delete query;

	for (auto &exchange : exchangePairs)
	{
		query = WrapperDb::instance()->select("select id, title from getPairsStrExchange(" + QString::number(exchange.exchange.first) + ");");

		//TODO: check query error

		while (query->next())
		{
			exchange.pairs.push_back({ query->value(0).toUInt(), query->value(1).toString() });
		}

		delete query;
	}
}

void GUICompare::initTable()
{

	using CompareExchange = std::pair<uint, uint>;
	std::vector<CompareExchange> fromTo;

	rows.clear();
	notification.clear();

	for (const auto &pair : currencyPair)
	{
		if (std::find(selectedPairs.begin(),selectedPairs.end(),pair.first) == selectedPairs.end()) continue;

		for (const auto &ep1 : exchangePairs)
		{
			if (std::find(ep1.pairs.begin(), ep1.pairs.end(), pair) == ep1.pairs.end() || (std::find(selectedExchange.begin(), selectedExchange.end(), ep1.exchange.first) == selectedExchange.end())) continue;

			for (const auto &ep2 : exchangePairs)
			{
				if (std::find(ep2.pairs.begin(), ep2.pairs.end(), pair) == ep2.pairs.end() || (std::find(selectedExchange.begin(), selectedExchange.end(), ep2.exchange.first) == selectedExchange.end())) continue;

				CompareExchange ce(ep1.exchange.first, ep2.exchange.first);

				if ((ep1.exchange.first == ep2.exchange.first) &&
					(std::find(fromTo.begin(), fromTo.end(), ce) == fromTo.end())) continue;
				
				rows.push_back({ ep1.exchange ,ep2.exchange , pair});

				fromTo.push_back(ce);

				const QString key(ep1.exchange.second + ep2.exchange.second + pair.second);
				notification[key] = false;
			}


		}

	}

	print();
}

void GUICompare::saveProperty()
{
	QString sql;

	QSettings setting(settingFile, QSettings::IniFormat);

	setting.beginGroup(propertyGroup);

		// save pair list
		QStringList list;
		for (const auto id : selectedPairs) list.append(QString::number(id));
		setting.setValue(propertyKeySelectedPair, list);

		sql += "select notice_compare_tool_set_pair('" + QString("{" + list.join(",") + "}") + "');";

		// save exchange list
		list.clear();
		for (const auto id : selectedExchange) list.append(QString::number(id));
		setting.setValue(propertyKeySelectedExchange, list);

		sql += "select notice_compare_tool_set_exchange('" + QString("{" + list.join(",") + "}") + "');";

		// save filter parameters 
		setting.beginGroup("filter");
			setting.setValue(propertyKeyActive,  ui.filter->isChecked());
			setting.setValue(propertyKeyPercent, ui.levelPercent->value());
			setting.setValue(propertyKeyVolume,  ui.levelVolume->value());
		setting.endGroup();

		sql += "select notice_compare_tool_set_level("+QString::number(ui.levelPercent->value() )+"," + QString::number(ui.levelVolume->value()) + ");";

		// save notification parameters
		setting.beginGroup("notification-compare");
			setting.setValue(propertyKeyActive,  ui.notification->isChecked());
			setting.setValue("template", ui.message->toPlainText());
		setting.endGroup();

		sql += "select notice_compare_tool_set_message('"+ ui.message->toPlainText() +"');";
		sql += "select enable_notice_compare_tool("+QString(ui.notification->isChecked() ? "true" : "false")+");";


		// save color indicator list
		QStringList percentList;
		QList<QVariant> colorList;
		for (int i = 0; i < ui.colorIndicator->rowCount(); ++i)
		{
			auto table = ui.colorIndicator;
			
			auto item = table->item(i, 0);
			if (!item) continue;
			const auto percent = item->data(Qt::DisplayRole).toDouble();

			item = table->item(i, 1);
			if (!item) continue;
			QColor color = item->data(Qt::DisplayRole).value<QColor>();

			item = table->item(i, 2);
			if (!item) continue;
			auto alpha = item->data(Qt::DisplayRole).toDouble();

			alpha = (alpha < 0) ? 0 : (alpha > 1) ? 1 : alpha;

			color.setAlphaF(alpha);

			percentList << QString::number(percent);
			colorList << color;
		}

		setting.setValue("color-indicator-percent", percentList);
		setting.setValue("color-indicator-color", colorList);


	setting.endGroup();
	
	auto query = WrapperDb::instance()->select(sql);

	if (query->lastError().isValid())
	{
		qDebug() << "Query error: " << query->lastError().databaseText();
	}
	
}

void GUICompare::loadProperty()
{
	QSettings setting(settingFile, QSettings::IniFormat);
		
	setting.beginGroup(propertyGroup);

		// selected pair
		selectedPairs.clear();
		for (const auto &idStr : setting.value(propertyKeySelectedPair).toStringList())
		{
			selectedPairs.push_back(idStr.toUInt());
		}
	
		for (auto row = 0; row < ui.tableCurrencyPair->rowCount(); ++row)
		{
			auto item = ui.tableCurrencyPair->item(row, 0);
			item->setCheckState(std::find(selectedPairs.begin(), selectedPairs.end(), item->data(Qt::UserRole + QVariant::UInt).toUInt()) == selectedPairs.end() ? Qt::Unchecked : Qt::Checked);
		}

		ui.selectAllPairs->setCheckState( (selectedPairs.size() == ui.tableCurrencyPair->rowCount()) ? Qt::Checked : Qt::Unchecked);

		// selected exchange
		selectedExchange.clear();
		for (const auto &idStr : setting.value(propertyKeySelectedExchange).toStringList())
		{
			selectedExchange.push_back(idStr.toUInt());
		}

		for (auto row = 0; row < ui.tableExchange->rowCount(); ++row)
		{
			auto item = ui.tableExchange->item(row, 0);
			item->setCheckState(std::find(selectedExchange.begin(),selectedExchange.end(),item->data(Qt::UserRole + QVariant::UInt).toUInt()) == selectedExchange.end() ? Qt::Unchecked : Qt::Checked );
		}

		ui.selectAllExchange->setCheckState( (selectedExchange.size() == ui.tableExchange->rowCount()) ? Qt::Checked : Qt::Unchecked);

		// for filter
		setting.beginGroup("filter");

			ui.filter->setChecked(setting.value(propertyKeyActive).toBool());
			ui.levelPercent->setValue(setting.value(propertyKeyPercent).toDouble());
			ui.levelVolume->setValue(setting.value(propertyKeyVolume).toDouble());

		setting.endGroup();
		
		// for notification
		setting.beginGroup("notification-compare");

			ui.notification->setChecked(setting.value(propertyKeyActive).toBool());

			const auto templateText = setting.value("template").toString();
			if (templateText.size())
			{
				ui.message->setPlainText(templateText);
			}

		setting.endGroup();


		// load color indicator list

		auto percentList = setting.value("color-indicator-percent").toStringList();
		auto colorList = setting.value("color-indicator-color").toList();

		const auto countColorIndicator = std::min(percentList.count(), colorList.count());

		ui.colorIndicator->setRowCount(countColorIndicator);

		ui.colorIndicator->setSortingEnabled(false);

		for (int i = 0; i < countColorIndicator; ++i)
		{
			const auto percent = percentList.at(i).toDouble();
			const auto color   = colorList.at(i).value<QColor>();

			auto table = ui.colorIndicator;

			auto percentItem = new QTableWidgetItem;
			auto colorItem   = new QTableWidgetItem;
			auto opacityItem = new QTableWidgetItem;

			percentItem->setData(Qt::DisplayRole, percent);
			colorItem->setData(Qt::DisplayRole, color);
			opacityItem->setData(Qt::DisplayRole, color.alphaF());

			table->setItem(i, 0, percentItem);
			table->setItem(i, 1, colorItem);
			table->setItem(i, 2, opacityItem);
		}

		ui.colorIndicator->setSortingEnabled(true);

	setting.endGroup();

	
}

void GUICompare::checkNotification(const QString &from, const QString &to, const QString &pair, double percent, const Entity::Order &ask, const Entity::Order &bid)
{
	const QString key(from + to + pair);
	if (!ui.notification->isChecked() || notification.find(key) == notification.end()) return;

	const auto availableVolume = std::min(ask.getAmount(), bid.getAmount());

	const auto levelPercent = ui.levelPercent->value();
	const auto levelVolume  = ui.levelVolume->value();
	
	const auto condition = levelPercent <= percent && levelVolume <= availableVolume;

	if (!notification[key] && condition)
	{
		QString msg = ui.message->toPlainText();

		msg.replace("$(from)",    from);
		msg.replace("$(to)",      to);
		msg.replace("$(pair)",    pair);
		msg.replace("$(current)", QString::number(percent,'f',2));
		msg.replace("$(buy)",     QString::number(ask.getPrice()));
		msg.replace("$(sell)",    QString::number(bid.getPrice()));
		msg.replace("$(volume)",  QString::number(availableVolume));

		qDebug() << msg;
		emit sendNotice(msg);
	}

	notification[key] = condition;
}

void GUICompare::print()
{
	ui.tableWidget->clearContents();

	ui.tableWidget->setRowCount(static_cast<int>( rows.size()) );
	ui.tableWidget->setSortingEnabled(false);
	
	using PercentColor = std::pair<double, QColor>;
	using VectorPercentColor = std::vector<PercentColor>;

	VectorPercentColor listColor;
	for (int i = 0; i < ui.colorIndicator->rowCount(); ++i)
	{
		auto table = ui.colorIndicator;
		PercentColor percentColor;
		auto item = table->item(i, 0);
		if (!item) continue;
		percentColor.first = item->data(Qt::DisplayRole).toDouble();

		item = table->item(i, 1);
		if (!item) continue;
		QColor color = item->data(Qt::DisplayRole).value<QColor>();

		item = table->item(i, 2);
		if (!item) continue;
		auto alpha = item->data(Qt::DisplayRole).toDouble();

		alpha = (alpha < 0) ? 0 : (alpha > 1) ? 1 : alpha;

		color.setAlphaF(alpha);

		percentColor.second = color;

		listColor.push_back(percentColor);
	}

	std::sort(listColor.begin(), listColor.end(), [](PercentColor a, PercentColor b) {return a.first < b.first; });

	auto rowCounter = 0;
	for (auto &row : rows)
	{
		auto column = 0;
		const auto bid = row.bid.getPrice();
		const auto ask = row.ask.getPrice();
		if (ask == 0.0) continue;

		const auto diff = bid - ask;
		const auto percent = diff / ask * 100;
		const auto availableVolume = std::min(row.ask.getAmount(), row.bid.getAmount());

		checkNotification(row.exchangeBuy.second, row.exchangeSell.second, row.pair.second, percent, row.ask, row.bid);

		const auto levelVolume = ui.levelVolume->value();
		if(ui.filter->isChecked() && (percent <= ui.levelPercent->value() || availableVolume <= levelVolume) ) continue;
		
		// color
		QColor color(Qt::white);
		color.setAlpha(0);
		for (const auto &percentColor : listColor)
		{
			if (percent >= percentColor.first) color = percentColor.second;
		}


		// from exchange
		auto item = new TableItem(row.exchangeBuy.second);

		QColor highlightText(item->textColor());
		bool highlightItem = false;
		if ((row.oldAsk.getPrice() != 0.0))
		{
			const auto oldBid = row.oldBid.getPrice();
			const auto oldAsk = row.oldAsk.getPrice();

			const auto oldPercent = (oldBid - oldAsk) / oldAsk * 100;

			highlightText = (oldPercent < percent) ? Qt::darkGreen : Qt::red;

			highlightItem = oldPercent != percent;
		}

		row.oldAsk = row.ask;
		row.oldBid = row.bid;

		auto font = item->font();
		font.setBold(std::abs(percent) >= 12);	

		const uint msecHighlight = 2000;
		
		item->setFont(font);
		if(color.isValid()) item->setBackgroundColor(color);

		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}
		

		// to exhcange
		item = new TableItem(row.exchangeSell.second);
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// Ask
		item = new TableItem;
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		item->setData(Qt::DisplayRole, ask);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// Bid
		item = new TableItem;
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		item->setData(Qt::DisplayRole, bid);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// Dif
		item = new TableItem;
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		item->setData(Qt::DisplayRole, diff);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// Percent
		item = new TableItem;
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		item->setData(Qt::DisplayRole, percent);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// Volume
		item = new TableItem;
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		item->setData(Qt::DisplayRole, availableVolume);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		// pair
		item = new TableItem(row.pair.second);
		item->setFont(font);
		if (color.isValid()) item->setBackgroundColor(color);
		ui.tableWidget->setItem(rowCounter, column++, item);
		if (highlightItem)
		{
			item->init();
			item->setTextHighlight(highlightText);
			item->highlight(msecHighlight);
		}

		rowCounter++;
	}

	ui.tableWidget->setRowCount(rowCounter);

	ui.tableWidget->setSortingEnabled(true);
}

void GUICompare::onCurrencyPairSelected(QTableWidgetItem * item)
{
	const auto id        = item->data(Qt::UserRole + QVariant::UInt).toUInt();
	const auto isChecked = item->checkState() == Qt::Checked;
	const auto itr       = std::find(selectedPairs.begin(), selectedPairs.end(), id);
	const auto inVector  = itr != selectedPairs.end();

	if (!inVector && isChecked)
	{
		selectedPairs.push_back(id);
	}
	else if(inVector && !isChecked)
	{
		selectedPairs.erase(itr);
	} 
	else
	{
		return;
	}

	ui.selectAllPairs->blockSignals(true);
	ui.selectAllPairs->setChecked(selectedPairs.size() == ui.tableCurrencyPair->rowCount());
	ui.selectAllPairs->blockSignals(false);

	initTable();
	saveProperty();
}

void GUICompare::onExchangeSelected(QTableWidgetItem * item)
{
	const auto id = item->data(Qt::UserRole + QVariant::UInt).toUInt();
	const auto isChecked = item->checkState() == Qt::Checked;
	const auto itr = std::find(selectedExchange.begin(), selectedExchange.end(), id);
	const auto inVector = itr != selectedExchange.end();

	if (!inVector && isChecked)
	{
		selectedExchange.push_back(id);
	}
	else if (inVector && !isChecked)
	{
		selectedExchange.erase(itr);
	}
	else
	{
		return;
	}

	ui.selectAllExchange->blockSignals(true);
	ui.selectAllExchange->setChecked(selectedExchange.size() == ui.tableExchange->rowCount());
	ui.selectAllExchange->blockSignals(false);

	initTable();
	saveProperty();
}

void GUICompare::onAllSelectedExchange(int state)
{
	if (Qt::CheckState(state) == Qt::Unchecked) selectedExchange.clear();
	ui.tableExchange->blockSignals(true);
	for (auto row = 0; row < ui.tableExchange->rowCount(); ++row)
	{
		auto item = ui.tableExchange->item(row, 0);
		item->setCheckState(Qt::CheckState(state));
		if (Qt::CheckState(state) == Qt::Checked) selectedExchange.push_back(item->data(Qt::UserRole + QVariant::UInt).toUInt());
	}
	ui.tableExchange->blockSignals(false);

	saveProperty();
	initTable();
}

void GUICompare::onAllSelectedPair(int state)
{
	if (Qt::CheckState(state) == Qt::Unchecked) selectedPairs.clear();
	ui.tableCurrencyPair->blockSignals(true);
	for (auto row = 0; row < ui.tableCurrencyPair->rowCount(); ++row)
	{
		auto item = ui.tableCurrencyPair->item(row, 0);
		item->setCheckState(Qt::CheckState(state));
		if (Qt::CheckState(state) == Qt::Checked) selectedPairs.push_back(item->data(Qt::UserRole + QVariant::UInt).toUInt());
	}
	ui.tableCurrencyPair->blockSignals(false);

	saveProperty();
	initTable();
}

void GUICompare::onAddColorIndicator()
{
	      auto table    = ui.colorIndicator;
	const auto countRow = table->rowCount();

	auto percent = new QTableWidgetItem;
	auto color   = new QTableWidgetItem;
	auto opacity = new QTableWidgetItem;

	percent->setData(Qt::DisplayRole, 0.0);
	color->setData(Qt::DisplayRole, QColor(Qt::white));
	opacity->setData(Qt::DisplayRole, 0.2);

	table->setSortingEnabled(false);

	table->setRowCount(countRow + 1);
	
	table->setItem(countRow, 0, percent);
	table->setItem(countRow, 1, color);
	table->setItem(countRow, 2, opacity);

	table->setSortingEnabled(true);
	
}

void GUICompare::onRemoveColorIndicator()
{
	auto selected = ui.colorIndicator->selectedItems();

	std::vector<int> r;
	for (const auto item : selected)
	{
		const auto row = ui.colorIndicator->row(item);

		if (std::find(r.begin(), r.end(), row) == r.end())
		{
			r.push_back(row);
		}
	}

	for (const auto row : r)
	{
		ui.colorIndicator->removeRow(row);
	}

}

void GUICompare::start()
{
	emit requestOrderBooks();
}