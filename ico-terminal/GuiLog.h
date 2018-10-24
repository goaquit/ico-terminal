#pragma once

#include <QWidget>
#include "ui_GuiLog.h"

using PairStringDate   = QPair<QString, QDateTime>;
using VectorStringDate = QVector<PairStringDate>;

class GuiLog : public QWidget
{
	Q_OBJECT

public:
	GuiLog(QWidget *parent = Q_NULLPTR);
	~GuiLog() = default;

	void setProfile(const QString &);

public slots:
	void onMessage(const QString &msg);

private:
	Ui::GuiLog ui;

	VectorStringDate readFromFile();
	void writeToFile(const PairStringDate &);

	void appendToTable(const PairStringDate &);

	QString profile  = "";
	QString fileName = "log.data";
};
