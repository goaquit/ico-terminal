#include "GuiLog.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QVector>
#include <QPair>
#include <QDataStream>
#include <QScrollBar>

GuiLog::GuiLog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void GuiLog::setProfile(const QString &p)
{
	profile = p;
	auto data = readFromFile();

	for (const auto &pair : data) appendToTable(pair);
}

void GuiLog::onMessage(const QString & msg)
{
	PairStringDate pair(msg, QDateTime::currentDateTimeUtc());

	appendToTable(pair);
	writeToFile(pair);
}

VectorStringDate GuiLog::readFromFile()
{
	VectorStringDate fromFile;

	if (!profile.size()) return fromFile;

	if (!QDir("profile").exists())
	{
		QDir().mkdir("profile");
	}

	const auto path("profile/" + profile);
	if (!QDir(path).exists())
	{
		QDir().mkdir(path);
	}

	const auto fn(path + "/" + fileName);
	QFile file(fn);

	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << fileName << " not open";
		return fromFile;
	}

	QDataStream streamFile(&file);

	streamFile >> fromFile;

	file.close();

	return fromFile;
}

void GuiLog::writeToFile(const PairStringDate &input)
{
	if (!profile.size()) return;

	if (!QDir("profile").exists())
	{
		QDir().mkdir("profile");
	}

	const auto path("profile/" + profile);
	if (!QDir(path).exists())
	{
		QDir().mkdir(path);
	}

	const auto fn(path + "/" + fileName);

	auto fromFile = readFromFile();
	while (fromFile.size() >= 100) fromFile.pop_front();

	fromFile.push_back(input);

	QFile file(fn);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qDebug() << fileName << " not open";
		return;
	}
	QDataStream streamFile(&file);

	streamFile << fromFile;

	file.close();

	

}

void GuiLog::appendToTable(const PairStringDate &pair)
{
	const auto row = ui.tableWidget->rowCount();
	ui.tableWidget->setRowCount(row + 1);

	ui.tableWidget->setItem(row, 0, new QTableWidgetItem(pair.first) );
	ui.tableWidget->setItem(row, 1, new QTableWidgetItem(pair.second.toString(Qt::ISODate)) );

	ui.tableWidget->scrollToBottom();
}
