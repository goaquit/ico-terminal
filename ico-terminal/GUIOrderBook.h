#pragma once

#include <QMainWindow>
#include "ui_GUIOrderBook.h"
#include <QtGui/QResizeEvent>
#include <QString>
#include "DepthOrder.h"

class ControllerOrderBook;

using OrderBook = Entity::DepthOrder;

namespace Terminal
{
	class GUIOrderBook : public QMainWindow
	{
		Q_OBJECT
	public:
		GUIOrderBook(QWidget *parent = Q_NULLPTR);
		~GUIOrderBook() = default;

		void setIndex(const QString&);

		void connectControllerOrderBook(const ControllerOrderBook *controllerOrderBook);

	public slots:
		void print() const;
		void changeDepth(int);
		void onOrderBookRecieved(const OrderBook &orderBook, const QString &index);

	protected:
		void resizeEvent(QResizeEvent *event) override;

	private:
		Ui::GUIOrderBook ui;

		QString index;
		OrderBook orderBook;
	};
}