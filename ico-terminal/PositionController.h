#pragma once

#include <QObject>
#include "Position.h"
#include "TradeApiWex.h"
#include <vector>

class Options;

class PositionController : public QObject
{
	Q_OBJECT

public:
	PositionController(QObject *parent = nullptr);

	void setProfile(const QString &);
	void setFileName(const QString &);

	void writeToFile() const;
	void readFromFile();	

	bool isReady() const;

	// getting position by hash
	Position * getPosition(const QByteArray &) const;

	inline std::vector<Position*>          getPositions() const {	return positions;	};
	inline QVector<QPair<QString, double>> getBalance()   const {	return balance;		}

	void setOptions(const Options *option);

public slots:
	void append(Position *);
	void remove(Position *);
	void cancelOrder(uint);

signals:
	void ready();
	void requestBalance();
	void newBalance(const QVector<QPair<QString, double>> &);
	void newActiveOrders(const QVector<TradeOrderInfo> &);

private:
	std::vector<Position*> positions;
	QVector<QPair<QString, double>> balance;

	QString profile	 = "";
	QString fileName = "position.data";

	TradeApiWex trade;
};
