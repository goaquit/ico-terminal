#include "Order.h"

namespace Entity
{

Order::Order(double p, double a) : price(p), amount(a){}

double Order::getPrice() const
{
	return price;
}

double Order::getAmount() const
{
	return amount;
}

void Order::setPrice(double value)
{
	price = value;
}

void Order::setAmount(double value)
{
	amount = value;
}

bool operator==(const Order & lhs, const Order & rhs)
{
	return (lhs.getPrice() == rhs.getPrice() && 
		lhs.getAmount() == rhs.getAmount());
}

bool operator!=(const Order & lhs, const Order & rhs)
{
	return !(lhs == rhs);
}
bool operator<(const Order & lhs, const Order & rhs)
{
	return lhs.getPrice() < rhs.getPrice();
}
bool operator>(const Order & lhs, const Order & rhs)
{
	return lhs.getPrice() > rhs.getPrice();
}
QDataStream & operator>>(QDataStream & stream, Order & order)
{
	double price  = 0.0;
	double amount = 0.0;

	stream >> price >> amount;

	order.setAmount(amount); order.setPrice(price);

	return stream;
}
QDataStream & operator<<(QDataStream & stream, const Order & order)
{
	stream << order.getPrice() << order.getAmount();
	return stream;
}


}