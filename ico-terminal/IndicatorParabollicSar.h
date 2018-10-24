#pragma once
#include "AbstractIndicator.h"

#include <QtCharts\QChartGlobal>

#include <deque>

QT_CHARTS_BEGIN_NAMESPACE
class QScatterSeries;
QT_CHARTS_END_NAMESPACE



namespace Indicators
{

struct SarData
{
	double SAR = 0.0;
	double EP = 0.0;
	double AF = 0.0;
	double HIP = 0.0;
	double LOP = 0.0;
	double isLong = true;
};

class IndicatorParabollicSar : public AbstractIndicator
{
public:
	IndicatorParabollicSar(AbstractIndicator *parent = nullptr);
	~IndicatorParabollicSar();

	// Inherited via AbstractIndicator
	virtual void process(const std::vector<Entity::TradePrice>&) override;
	virtual void append(const Entity::TradePrice &) override;
	virtual void removeFirst() override;
	virtual void removeLast() override;
	virtual void clear() override;

	QAbstractSeries *getSeries() const;

	void setMax(double);
	void setStep(double);

	// getting parameters
	double getMax() const;
	double getStep() const;

	QColor getColor() const;
	double getSize() const;

	// changes parameters
	void changeMax(double);
	void changeStep(double);

	void changeColor(const QColor &);
	void changeSize(double);

private:
	double step = 0.02;
	double maxAf  = 0.2;

	double SAR = 0.0;
	double EP  = 0.0;
	double AF  = 0.0;

	double HIP = 0.0;
	double LOP = 0.0;
	
	bool isLong = true;

	SarData backupData = {};

	Entity::TradePrice prevTrade;
	Entity::TradePrice saveTrade;

	void detectEntered();

	uint detectEnteredPeriod = 10;
	std::deque<Entity::TradePrice> previous;

	QScatterSeries *series = nullptr;
};


}