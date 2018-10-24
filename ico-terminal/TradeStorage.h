#pragma once
#include "AbstractIndicator.h"
#include "TradePrice.h"
#include <QObject>

QT_BEGIN_NAMESPACE
class QTimer;
class QMutex;
QT_END_NAMESPACE

using namespace Entity;

namespace DataStorage
{


class TradeStorage : public QObject
{
	Q_OBJECT

public:
	explicit TradeStorage(QObject *parent = nullptr);
	~TradeStorage();

	void reset();

	void setExchange(uint);
	void setPair(uint);
	void setSegment(int);
	void setTimeFrame(uint);

	uint getSegment() const;
	uint getTimeframe() const;

	const QString getTimeframeText() const;

	void connectIndicator(Indicators::AbstractIndicator * indicator);

public slots:
	void fullDataRequest();
	void run();
	void finish();
	void setEndTime(uint);
	void disabledUserSegment();
	void changeSegment(uint);
	void changeTimeFrame(uint);
	void onDateInfo(uint);

signals:
	void fullData(const QList<TradePrice> &);
	void sendNewData(const QList<TradePrice> &);
	// change
	void changedLastTrade(const TradePrice &);
	void sendDateInfo(const QString &);

private:
	QList<TradePrice> data;
	TradePrice lastTrade;

	QTimer *timer = nullptr;

	uint timeframe = 900;
	int segment   = 86400;
	
	uint endTime;
	uint startTime;
	bool userSegment = false;
	bool isFIrstLoad = true;

	uint exchange = 0;
	uint pair     = 0;

	QMutex *mtx = nullptr;

private slots:
	void loadData();
};


}