#include "DepthOrder.h"
#include <algorithm>
#include <QVector>

namespace Entity
{
void DepthOrder::addAsk(const Order &order)
{
	vectorAsks.push_back(order);
	qntAsks += order.getAmount();
}

void DepthOrder::addBid(const Order &order)
{
	vectorBids.push_back(order);
	qntBids += order.getAmount();
}

std::vector<Order> DepthOrder::asks() const
{
	return vectorAsks;
}

std::vector<Order> DepthOrder::bids() const
{
	return vectorBids;
}

double DepthOrder::quantityAsks() const
{
	return qntAsks;
}

double DepthOrder::quantityBids() const
{
	return qntBids;
}

void DepthOrder::setQuantityAsks(double value)
{
	qntAsks = value;
}

void DepthOrder::setQuantityBids(double value)
{
	qntBids = value;
}

void DepthOrder::sort()
{
	std::sort(vectorAsks.begin(), vectorAsks.end());
	std::sort(vectorBids.begin(), vectorBids.end());
}

DepthOrder DepthOrder::getLimit(const uint & limit) const
{
	DepthOrder order;

	order.qntAsks = qntAsks;
	order.qntBids = qntBids;

	const auto limitAsk = vectorAsks.size() < limit ? vectorAsks.size() : limit;
	const auto limitBid = vectorBids.size() < limit ? vectorBids.size() : limit;

	auto itrEnd = vectorAsks.begin();
	std::advance(itrEnd, limitAsk);
	for (auto itr = vectorAsks.begin(); itr != itrEnd; ++itr)
	{
		order.vectorAsks.push_back(*itr);
	}


	itrEnd = vectorBids.begin();
	std::advance(itrEnd, limitBid);
	for (auto itr = vectorBids.begin(); itr != itrEnd; ++itr)
	{
		order.vectorBids.push_back(*itr);
	}

	//TODO: not work
	/*auto itrEnd = vectorAsks.begin();
	std::advance(itrEnd, limitAsk + 1);
	order.vectorAsks.reserve(limitAsk);
	std::copy(vectorAsks.begin(), itrEnd, order.vectorAsks.begin());*/

	/*itrEnd = vectorBids.begin();
	std::advance(itrEnd, limitBid);
	order.vectorBids.reserve(limitBid + 1);
	std::copy(vectorBids.begin(), itrEnd, order.vectorBids.begin());*/

	return order;
}

QDataStream & operator<<(QDataStream & stream, const DepthOrder & depthOrder)
{
	auto asks = QVector<Order>::fromStdVector(depthOrder.asks());
	auto bids = QVector<Order>::fromStdVector(depthOrder.bids());

	stream << depthOrder.quantityAsks() << depthOrder.quantityBids() << asks << bids;

	return stream;
}

QDataStream & operator>>(QDataStream & stream, DepthOrder & depthOrder)
{
	QVector<Order> asks, bids;

	double quantityAsks, quantityBids;

	stream >> quantityAsks >> quantityBids >> asks >> bids;

	depthOrder.setQuantityAsks(quantityAsks);
	depthOrder.setQuantityBids(quantityBids);

	depthOrder.setAsks(asks.toStdVector());
	depthOrder.setBids(bids.toStdVector());

	return stream;
}

}