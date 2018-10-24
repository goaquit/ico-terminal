#include <algorithm>
#include "Calculation.h"
#include "IndicatorDx.h"
#include <QtCharts\QLineSeries>

using namespace Entity;

double SMA(const std::vector<double> &input, unsigned period)
{
	if (!period) return (input.size()) ? *input.rend() : 0;

	double sum = 0.0;
	auto n = period;
	for (auto itr = input.crbegin(); itr != input.crend() && n > 0; ++itr, --n)
	{
		sum += *itr;
	}

	return sum / period;
}

namespace Indicators
{


IndicatorDx::IndicatorDx(AbstractIndicator * parent) : 
	AbstractIndicator(parent),
	adx(new QLineSeries),
	pdi(new QLineSeries),
	ndi(new QLineSeries)
{
	seriesList.push_back(adx);
	seriesList.push_back(pdi);
	seriesList.push_back(ndi);


	adx->setPen(QPen(Qt::yellow,2));
	pdi->setPen(QPen(Qt::red, 2));
	ndi->setPen(QPen(QColor(0,200,255),2));

	type = AbstractIndicator::Adx;
	alias = "Adx";
}

IndicatorDx::~IndicatorDx()
{
	clear();

	adx->deleteLater();
	pdi->deleteLater();
	ndi->deleteLater();
}


void IndicatorDx::process(const std::vector<TradePrice> &input)
{
	if (input.size() < 2 || !period) return;

	clear();

	for (auto itr = ++input.cbegin(); itr != input.cend(); ++itr)
	{
		append(*itr, *(itr - 1));
	}
}

void IndicatorDx::append(const TradePrice & current, const TradePrice & prev)
{
	if (!period) return;

	const auto tr_i = std::max(current.getMaxPrice(), prev.getClosePrice()) - std::min(current.getMinPrice(), prev.getClosePrice());

	tr.push_back(tr_i);

	const auto pm = current.getMaxPrice() - prev.getMaxPrice();
	const auto nm = prev.getMinPrice() - current.getMinPrice();

	const auto pdm_i = ((pm > nm) && (pm > 0)) ? pm : 0.0;
	const auto ndm_i = ((nm > pm) && (nm > 0)) ? nm : 0.0;

	pdm.push_back(pdm_i);
	ndm.push_back(ndm_i);

	const auto atr_i = Calculation::Sma(tr, period);

	const auto pdi_i = (atr_i == 0.0) ? 0 : (Calculation::Sma(pdm, period) / atr_i) * 100;
	const auto ndi_i = (atr_i == 0.0) ? 0 : (Calculation::Sma(ndm, period) / atr_i) * 100;

	const auto di_iSum = pdi_i + ndi_i;
	const auto dx_i = (di_iSum == 0.0) ? 0.0 : (std::abs(pdi_i - ndi_i) / di_iSum) * 100;
	dx.push_back(dx_i);

	const auto adx_i = Calculation::Sma(dx, period);

	adx->append(current.getDate() * 1000, adx_i);
	pdi->append(current.getDate() * 1000, pdi_i);
	ndi->append(current.getDate() * 1000, ndi_i);
}

void IndicatorDx::removeLast()
{
	const int size = dx.size();
	
	if (!size) return;
	
	if (size == 1) clear();

	dx.pop_back();
	pdm.pop_back();
	ndm.pop_back();
	tr.pop_back();

	adx->removePoints(size - 1, 1);
	pdi->removePoints(size - 1, 1);
	ndi->removePoints(size - 1, 1);
}

void IndicatorDx::removeFirst()
{
	const auto size = dx.size();
	if (!size) return;
	if (size == 1) clear();

	dx.pop_front();
	pdm.pop_front();
	ndm.pop_front();
	tr.pop_front();

	adx->removePoints(0, 1);
	pdi->removePoints(0, 1);
	ndi->removePoints(0, 1);
}

void IndicatorDx::clear()
{
	dx.clear();
	pdm.clear();
	ndm.clear();
	tr.clear();

	adx->clear();
	pdi->clear();
	ndi->clear();

	lastTrade.clear();
}

void IndicatorDx::setPeriod(uint n)
{
	period = n;
}

uint IndicatorDx::getPeriod() const
{
	return period;
}

QColor IndicatorDx::getAdxColor() const
{
	return adx->color();
}

QColor IndicatorDx::getPdiColor() const
{
	return pdi->color();
}

QColor IndicatorDx::getNdiColor() const
{
	return ndi->color();
}

int IndicatorDx::getAdxSize() const
{
	return adx->pen().width();
}

int IndicatorDx::getPdiSize() const
{
	return pdi->pen().width();
}

int IndicatorDx::getNdiSize() const
{
	return ndi->pen().width();
}

void IndicatorDx::changePeriod(uint p)
{
	setPeriod(p);
	
	emit requestData();
}

void IndicatorDx::changeAdxColor(const QColor &c)
{
	adx->setColor(c);
}

void IndicatorDx::changePdiColor(const QColor &c)
{
	pdi->setColor(c);
}

void IndicatorDx::changeNdiColor(const QColor &c)
{
	ndi->setColor(c);
}

void IndicatorDx::changeAdxSize(int s)
{
	auto p = adx->pen();
	p.setWidth(s);
	adx->setPen(p);
}

void IndicatorDx::changePdiSize(int s)
{
	auto p = pdi->pen();
	p.setWidth(s);
	pdi->setPen(p);
}

void IndicatorDx::changeNdiSize(int s)
{
	auto p = ndi->pen();
	p.setWidth(s);
	ndi->setPen(p);
}

void IndicatorDx::append(const Entity::TradePrice &trade)
{
	if (!lastTrade.isEmpty())
	{
		append(trade, lastTrade);
	}

	lastTrade = trade;
}

//void IndicatorDx::onReceivingNewData(const QList<Entity::TradePrice> &tradeList)
//{
//	for (const auto &trade : tradeList)
//	{
//		append(trade);
//
//		lastTrade = trade;
//	}
//
//
//}
//
//void IndicatorDx::onAppendData(const Entity::TradePrice &trade)
//{
//	append(trade);
//
//	lastTrade = trade;
//}


}