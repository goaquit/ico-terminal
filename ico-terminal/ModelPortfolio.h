#pragma once
#include <QAbstractTableModel>
#include "PortfolioRow.h"
#include <vector>
#include "DepthOrder.h"

using OrderBook = Entity::DepthOrder;

class ModelPortfolio : public QAbstractTableModel
{
	Q_OBJECT

public:
	ModelPortfolio(QObject *parent = nullptr);

	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
		    
	virtual int columnCount(const QModelIndex & parent = QModelIndex()) const override;
		    
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

	virtual QVariant ModelPortfolio::headerData(int section, Qt::Orientation orientation, int role) const override;

	void changePortfolioRow(const PortfolioRow &row);
	void changeCurrencyTotal(const QString &currency);

signals:
	void requestAllOrderBook();
	void changedTotal(double);

public slots:
	void onOrderBookRecieved(const OrderBook &orderBook, const QString &index);

private:
	std::vector<PortfolioRow> grid;
	QString currencyTotal = "";
};

