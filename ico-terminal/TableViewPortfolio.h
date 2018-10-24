#pragma once
#include <QTableView>

namespace Entity
{
class Balance;
}
class ControllerOrderBook;
class GuiTrade;

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

class TableViewPortfolio : public QTableView
{
	Q_OBJECT
public:
	TableViewPortfolio(QWidget *parent = nullptr);

	void connectControllerOrderBook(const ControllerOrderBook *);
	void connectBalance(const Entity::Balance *);

protected:
	void contextMenuEvent(QContextMenuEvent *e) override;

private:
	QAction * actionTrade;

	void onTrade();

	void initAction();

	GuiTrade *windowTrade = nullptr;
};

