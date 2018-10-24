#include "ModelPortfolio.h"
#include <QString>

// column: 
// - exchange
// - pair
// - price
// - volume
// - total
static const int const_columnCount = 5;

ModelPortfolio::ModelPortfolio(QObject *parent) : QAbstractTableModel(parent) {}

int ModelPortfolio::rowCount(const QModelIndex & /*parent*/) const
{
	return grid.size();
}

int ModelPortfolio::columnCount(const QModelIndex & /*parent*/) const
{
	//todo: controller column
	return const_columnCount;
}

QVariant ModelPortfolio::data(const QModelIndex &index, int role) const
{
	// for DisplayRole
	if (role == Qt::DisplayRole)
	{
		const auto row = index.row();
		QString display("%1");

		QString result = "";

		switch (index.column())
		{
			case 0: result = display.arg(grid.at(row).getExchange());	break;
			case 1: result = display.arg(grid.at(row).getSymbol());		break;
			case 2: result = display.arg(grid.at(row).getPrice());		break;
			case 3: result = display.arg(grid.at(row).getVolume());		break;
			case 4: result = display.arg(grid.at(row).getTotal());		break;
			default:
				result;
				break;
		}

		return result;
	}

	return QVariant();
}

QVariant ModelPortfolio::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case 0:	return QString("exchange");
			case 1:	return QString("symbol");
			case 2:	return QString("price");
			case 3:	return QString("volume");
			case 4:	return QString("total");
			}
		}
	}
	return QVariant();
}

void ModelPortfolio::changePortfolioRow(const PortfolioRow & row)
{
	int counter = 0;
	bool changed = false;
	for (auto itr = grid.begin(); itr != grid.end(); ++itr, ++counter)
	{
		if (*itr != row) continue;		

		if (row.getVolume() <= 0.000001)
		{
			beginRemoveRows(createIndex(0, 0).parent(), counter, counter);
			grid.erase(itr);
			endRemoveRows();
		}
		else
		{
			itr->setVolume(row.getVolume());

			const auto index = createIndex(counter, 2);
			emit dataChanged(index, index);
		}

		changed = true;
		break;
	}

	if (!changed)
	{
		if (row.getVolume() <= 0.000001) return;

		const auto size = grid.size();
		beginInsertRows(createIndex(0, 0).parent(), size, size);

		grid.push_back(row);

		endInsertRows();
	}
}

void ModelPortfolio::changeCurrencyTotal(const QString & currency)
{
	currencyTotal = currency;

	emit requestAllOrderBook();
}

void ModelPortfolio::onOrderBookRecieved(const OrderBook & orderBook, const QString & index)
{
	if (currencyTotal.isEmpty()) return;

	int counterRow = 0;

	double totalCurrency = 0.0;

	for (auto row = grid.begin(); row != grid.end(); ++row, ++counterRow)
	{
		const auto begin = createIndex(counterRow, 0);
		const auto end = createIndex(counterRow, counterRow - 1);

		if (!currencyTotal.compare(row->getSymbol(), Qt::CaseInsensitive))
		{
			row->setPrice(1.0);
			row->setTotal(row->getVolume());

			emit dataChanged(begin, end);

			totalCurrency += row->getTotal();

			continue;
		}
		if (orderBook.asks().empty()) continue;

		const auto price = orderBook.asks().begin()->getPrice();

		const auto direct  = row->getExchange() + "_" + row->getSymbol() + currencyTotal;
		const auto inverse = row->getExchange() + "_" + currencyTotal    + row->getSymbol();

		if (!direct.compare(index, Qt::CaseInsensitive))
		{		
			const auto total = price * row->getVolume();
			row->setPrice(price);
			row->setTotal(total);

			emit dataChanged(begin, end);
		}
		else if (!inverse.compare(index, Qt::CaseInsensitive))
		{
			const auto total = row->getVolume() / price;
			row->setPrice(price);
			row->setTotal(total);

			emit dataChanged(begin, end);
		}	

		totalCurrency += row->getTotal();
	}

	emit changedTotal(totalCurrency);
}


