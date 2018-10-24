#pragma once

#include "Order.h"
#include <vector>
#include <QDataStream>

namespace Entity
{


class DepthOrder
{
public:
	void addAsk(const Order&);
	void addBid(const Order&);

	std::vector<Order> asks() const;
	std::vector<Order> bids() const;

	double quantityAsks() const;
	double quantityBids() const;

	void setQuantityAsks(double);
	void setQuantityBids(double);

	inline void setAsks(const std::vector<Order> &volume) {	vectorAsks = volume;};
	inline void setBids(const std::vector<Order> &volume) {	vectorBids = volume;};

	void sort();

	DepthOrder getLimit(const uint &limit) const;

private:

	std::vector<Order> vectorAsks;
	std::vector<Order> vectorBids;

	double qntAsks = 0;
	double qntBids = 0;
};


QDataStream &operator<<(QDataStream &stream, const DepthOrder &depthOrder);
QDataStream &operator>>(QDataStream &stream, DepthOrder &depthOrder);
}