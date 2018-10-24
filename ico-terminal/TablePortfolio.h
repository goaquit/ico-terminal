#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_TablePortfolio.h"
#include "ModelPortfolio.h"

namespace Entity
{
class Balance;
}

class ControllerOrderBook;

class TablePortfolio : public QMainWindow
{
	Q_OBJECT

public:
	TablePortfolio(QWidget *parent = Q_NULLPTR);

	void connectBalance(const Entity::Balance *);
	void connectControllerOrderBook(const ControllerOrderBook *);

private:
	Ui::TablePortfolioClass ui;
	ModelPortfolio model;

	void initConnect();

private slots:
	void onSelectCurrency(const QString &text);
};
