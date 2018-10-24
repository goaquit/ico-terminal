#pragma once
#include "AbstractIndicator.h"

#include <QtCharts\QChartGlobal>

#include <ctime>
#include <deque>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE
using namespace Entity;

namespace Indicators
{

class IndicatorDx : public AbstractIndicator
{
public:

	IndicatorDx(AbstractIndicator *parent = nullptr);
	~IndicatorDx();

	// Inherited via AbstractIndicator
	virtual void append(const Entity::TradePrice &) override;
	        void append(const TradePrice & current, const TradePrice & prev);
	virtual void clear() override;
	virtual void process(const std::vector<TradePrice> &) override;
	virtual void removeFirst() override;
	virtual void removeLast()  override;

	void setPeriod(uint);

	// get parameters
	uint getPeriod() const;
	QColor getAdxColor() const;
	QColor getPdiColor() const;
	QColor getNdiColor() const;

	int getAdxSize() const;
	int getPdiSize() const;
	int getNdiSize() const;

	// change parameters
	void changePeriod(uint);
	
	void changeAdxColor(const QColor &);
	void changePdiColor(const QColor &);
	void changeNdiColor(const QColor &);

	void changeAdxSize(int);
	void changePdiSize(int);
	void changeNdiSize(int);

private:
	std::deque<double> pdm;
	std::deque<double> ndm;

	std::deque<double> tr;

	std::deque<double> dx;

	//data
	Entity::TradePrice lastTrade;
	uint period = 0;

	// series
	QLineSeries *adx = nullptr;
	QLineSeries *pdi = nullptr;
	QLineSeries *ndi = nullptr;
};


}