#pragma once

#include "TradePrice.h"

#include <QObject>

using namespace Entity;

namespace Terminal
{

	class AbstractTradeLoader : public QObject
	{
		Q_OBJECT

	public:
		void setExchange(uint32_t);
		void setPair(uint32_t);

		virtual QList<TradePrice> loadData(time_t start, time_t end, time_t timeFrame) = 0;

	public slots:
		void run(quint64 start, quint64 end, quint64 timeFrame);
		void requestData() const;

	signals:
		void sendDate(QList<TradePrice>);
		void newData(const QList<TradePrice> &);
		void changeLastData(const TradePrice &);
		void sendFullData(const QList<TradePrice> &);

	protected:

		uint32_t exchangeId = 0;
		uint32_t pair = 0;

		QList<TradePrice> listTradePrice;
	};
}