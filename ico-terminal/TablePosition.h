#pragma once

#include <QObject>
#include <QTableWidget>

class TablePosition : public QTableWidget
{
	Q_OBJECT

public:
	TablePosition(QWidget *parent = nullptr);
	~TablePosition() = default;

#ifndef QT_NO_CONTEXTMENU
	void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

	inline void setAdd(QAction *action)		  {	addPosition = action;	};
	inline void setClose(QAction *action)	  {	closePosition = action;	};
	inline void setRemove(QAction *action)	  {	removePosition = action;};
	inline void setModify(QAction *action)	  {	modifyPosition = action;};
	inline void setOrderBook(QAction *action) {	openOrderBook = action;	};
	inline void setChart(QAction *action)	  {	openChart = action;		};

private:
	// action
	void initAction();
	QAction *addPosition = nullptr;
	QAction *closePosition = nullptr;
	QAction *removePosition = nullptr;
	QAction *modifyPosition = nullptr;
	QAction *openOrderBook = nullptr;
	QAction *openChart = nullptr;
};
