#pragma once
#include "TradePrice.h"

#include <QtCharts\QChartGlobal>
#include <QObject>
#include <QUuid>
#include <vector>



QT_CHARTS_BEGIN_NAMESPACE
class QAbstractSeries;
QT_CHARTS_END_NAMESPACE

namespace ChartCore
{
class Chart;
}

QT_CHARTS_USE_NAMESPACE

namespace Indicators
{


class AbstractIndicator : public QObject
{
	Q_OBJECT
public:
	explicit AbstractIndicator(QObject *parent = nullptr);
	~AbstractIndicator();
	
	enum Type
	{
		Adx,
		Bollinger,
		Sar,
		Sma,
		Stochastic
	};

	virtual void process(const std::vector<Entity::TradePrice> &) = 0;
	virtual void append(const Entity::TradePrice &) = 0;

	virtual void removeFirst() = 0;
	virtual void removeLast() = 0;
	virtual void clear() = 0;

	void setChart(ChartCore::Chart *);
	void setAlias(const QString &);

	inline void setUuid(const QUuid &u) {	uuid = u;	};
	inline QUuid getUuid() const {	return uuid;	}

	inline QString getAlias() const {	return alias;	}
	inline Type    getType()  const {	return type;	}

public slots:
	virtual void onReceivingFullData(const QList<Entity::TradePrice> &tradeList);
	virtual void onReceivingNewData(const QList<Entity::TradePrice> &tradeList);
	virtual void onAppendData(const Entity::TradePrice &trade);
	virtual void onChangeLastData(const Entity::TradePrice &trade);

signals:
	void requestData();

protected:
	QUuid uuid;

	std::vector<QAbstractSeries *> seriesList;

	QString alias;

	Type type;
};


}