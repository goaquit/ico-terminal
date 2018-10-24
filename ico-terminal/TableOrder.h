#pragma once

#include <QTableWidget>

class TableOrder : public QTableWidget
{
	Q_OBJECT

public:
	TableOrder(QWidget *parent);
	~TableOrder() = default;

#ifndef QT_NO_CONTEXTMENU
	void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

	inline void setCancel(QAction *action) {	closeOrder = action;	};

private:
	// action
	void initAction();
	QAction *closeOrder = nullptr;
};
