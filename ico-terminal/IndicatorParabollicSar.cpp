#include "Calculation.h"
#include "IndicatorParabollicSar.h"
#include <QtCharts\QScatterSeries>

namespace Indicators
{


IndicatorParabollicSar::IndicatorParabollicSar(AbstractIndicator * parent) :
	AbstractIndicator(parent) ,series(new QScatterSeries(this))
{
	seriesList.push_back(series);

	series->setPen(QPen());
	series->setMarkerSize(5);

	type = AbstractIndicator::Sar;
	alias = "Parabollic SAR";
}

IndicatorParabollicSar::~IndicatorParabollicSar()
{
	series->deleteLater();
}

void IndicatorParabollicSar::process(const std::vector<Entity::TradePrice>&)
{
}

void IndicatorParabollicSar::append(const Entity::TradePrice &trade)
{
	if (previous.size() < detectEnteredPeriod)
	{
		previous.push_back(trade);
		
		if (previous.size() < detectEnteredPeriod)
		{
			prevTrade = trade;
			return;
		}

		detectEntered();
	}


	//if (SAR == 0.0)
	//{
	//	SAR = trade.getMinPrice();
	//	LOP = trade.getMinPrice();
	//	HIP = trade.getMaxPrice();

	//	prevTrade = trade;

	//	isLong = true;

	//	return;
	//}

	backupData.SAR    = SAR;
	backupData.EP     = EP;
	backupData.AF     = AF;
	backupData.HIP    = HIP;
	backupData.LOP    = LOP;
	backupData.isLong = isLong;

	saveTrade = prevTrade;


	if (isLong && (SAR > trade.getMinPrice() || SAR > prevTrade.getMinPrice()))
	{
		SAR = HIP;
		AF = step;
		isLong = !isLong;

		HIP = std::max(prevTrade.getMaxPrice(), trade.getMaxPrice());
		LOP = std::min(prevTrade.getMinPrice(), trade.getMinPrice());
	}
	else if (!isLong && (SAR < trade.getMaxPrice() || SAR < prevTrade.getMaxPrice()))
	{
		SAR = LOP;
		AF = step;
		isLong = !isLong;

		HIP = std::max(prevTrade.getMaxPrice(), trade.getMaxPrice());
		LOP = std::min(prevTrade.getMinPrice(), trade.getMinPrice());
	}

	series->append(trade.getDate() * 1000, SAR);

	const double ep = (isLong) ? HIP : LOP;

	SAR = Calculation::Sar(SAR, ep, AF, isLong);


	if( (AF < maxAf) && ((isLong && HIP < trade.getMaxPrice()) || (!isLong && LOP > trade.getMinPrice())))
	{
		AF += step;
	}

	HIP = std::max(HIP,trade.getMaxPrice());
	LOP = std::min(LOP,trade.getMinPrice());

	prevTrade = trade;
}

void IndicatorParabollicSar::removeFirst()
{
	const auto size = series->points().size();

	if (size < 2)
	{
		clear();
		
		return;
	}

	series->removePoints(0, 1);
}

void IndicatorParabollicSar::removeLast()
{
	const auto size = series->points().size();

	if (size < 2)
	{
		clear();

		return;
	}

	series->removePoints(size - 1, 1);

	prevTrade = saveTrade;

	SAR = backupData.SAR;
	EP = backupData.EP;
	AF = backupData.AF;
	HIP = backupData.HIP;
	LOP = backupData.LOP;
	isLong = backupData.isLong;

	backupData = {};
	saveTrade.clear();
}

void IndicatorParabollicSar::clear()
{
	series->clear();

	AF  = step;
	EP  = 0.0;
	SAR = 0.0;

	HIP = 0.0;
	LOP = 0.0;

	previous.clear();
	prevTrade.clear();
	saveTrade.clear();
}

QAbstractSeries * IndicatorParabollicSar::getSeries() const
{
	return series;
}

void IndicatorParabollicSar::setMax(double v)
{
	maxAf = v;
}

void IndicatorParabollicSar::setStep(double v)
{
	step = v;
}

double IndicatorParabollicSar::getMax() const
{
	return maxAf;
}

double IndicatorParabollicSar::getStep() const
{
	return step;
}

QColor IndicatorParabollicSar::getColor() const
{
	return series->color();
}

double IndicatorParabollicSar::getSize() const
{
	return series->markerSize();
}

void IndicatorParabollicSar::changeMax(double v)
{
	setMax(v);
	emit requestData();
}

void IndicatorParabollicSar::changeStep(double v)
{
	setStep(v);
	emit requestData();
}

void IndicatorParabollicSar::changeColor(const QColor &v)
{
	series->setColor(v);
}

void IndicatorParabollicSar::changeSize(double v)
{
	series->setMarkerSize(v);
}


void IndicatorParabollicSar::detectEntered()
{
	auto itr = previous.cbegin();
	double max = itr->getMaxPrice();
	double min = itr->getMinPrice();

	SAR = min;

	for (; itr != previous.cend(); ++itr)
	{
		const auto localMax = itr->getMaxPrice();
		const auto localMin = itr->getMinPrice();
		if (localMin < min || localMax > max)
		{
			if (localMin < min)
			{
				min = localMin;
				
				SAR = min;
				isLong = true;

				LOP = min;
				HIP = localMax;
			} else if (localMax > max)
			{
				max = localMax;

				SAR = max;
				isLong = false;

				LOP = localMin;
				HIP = max;
			}
		}
	}
}

}