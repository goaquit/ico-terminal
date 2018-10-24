#pragma once

#include <QTableWidget>
#include <QTimer>

class TableItem : public QTableWidgetItem
{
public:
	TableItem(int type = Type);
	TableItem(const QString &text, int type = Type);
	~TableItem() override;

	void highlight(uint msec = 0);

	void setBackgroundHighlight(const QColor &);
	void setTextHighlight(const QColor &);

	void init();

private:
	QColor backgroundHighlight;
	QColor textHighlight;

	bool isHighlighted = false;

	bool stoped = false;

	QTimer timer;
};

