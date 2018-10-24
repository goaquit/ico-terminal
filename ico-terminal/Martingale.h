#pragma once

#include <QObject>

class Martingale : public QObject
{
	Q_OBJECT

public:
	Martingale(QObject *parent = nullptr);
	~Martingale() = default;

	inline double getVolume() const {	return volume;	}
	inline double getPrice()  const {	return price;	}
	inline double getCost()   const {	return cost;	}

	bool isActive() const {	return isRunning && !isActiveOrder;}
	bool hasRunning() const { return isRunning;}

	void setPair(const QString &pair);

signals:
	void buy(double volume, double price, const QString &pair);
	void sell(double volume, double price, const QString &pair);
	void finished();
	void profit(double);

public slots:
	void start(double volume, double price);
	void stop();
	void stop(double price);
	void check(double buy, double sell);
	void onBuy(double volume, double price);
	void onSell(double volume, double price);

private:
	double volume = 0.0;
	double price  = 0.0;
	double cost   = 0.0;

	double commissionRate = 0.002;

	bool isRunning = false;
	bool isActiveOrder = false;

	QString pair;

	void processingNewMiddlePrice(double price);
	void clear();
};
