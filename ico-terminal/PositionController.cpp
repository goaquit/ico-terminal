#include "PositionController.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include "TradeApiWex.h"
#include <QVector>
#include <QCryptographicHash>
#include <memory>
#include <QTimer>
#include "Options.h"

PositionController::PositionController(QObject *parent)
	: QObject(parent)
{
	QObject::connect(&trade, &TradeApiWex::synchronized, [this]()
	{
		for (auto position : positions)
		{
			position->setTradeApi(trade);
		}

		emit ready();

		trade.getInfo();
		trade.activeOrders();
	});
	QObject::connect(&trade, &TradeApiWex::newBalance, [this](const QVector<QPair<QString, double>> &b)
	{
		balance = b;
		emit newBalance(balance);
	});

	QObject::connect(&trade, &TradeApiWex::newActiveOrders, this, &PositionController::newActiveOrders);

	qRegisterMetaType<Position>("Position");
	qRegisterMetaType<TradeOrderInfo>("TradeOrderInfo");
}

void PositionController::append(Position *position)
{
	positions.push_back(position);
	position->setTradeApi(trade);

	writeToFile();

	QObject::connect(position, &Position::requestToSave, [this]()
	{
		writeToFile();
	});

	QObject::connect(position, &Position::removing, this, &PositionController::remove);
}

void PositionController::setOptions(const Options * option)
{
	trade.setKey(option->getWexKey());
	trade.setSecret(option->getWexSecret());

	trade.synchronize();

	auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]()
	{
		trade.getInfo();
	});

	timer->start(30000);

	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]()
	{
		trade.activeOrders();
	});

	timer->start(15000);
}

void PositionController::remove(Position *position)
{
	if (!position) return;
	auto itr = std::remove(positions.begin(), positions.end(), position);
	if (itr == positions.end()) return;

	positions.erase(itr);

	connect(position, &Position::readyToDelete, position, &Position::deleteLater);

	position->cancelAllOrders();

	writeToFile();
}

void PositionController::cancelOrder(uint id)
{
	trade.cancelOrder(id);
}

void PositionController::setProfile(const QString &v)
{
	profile = v;
	readFromFile();
}

void PositionController::setFileName(const QString &v)
{
	fileName = v;
}

void PositionController::writeToFile() const
{
	if (!profile.size()) return;

	if (!QDir("profile").exists())
	{
		QDir().mkdir("profile");
	}

	const auto path("profile/" + profile);
	if (!QDir(path).exists())
	{
		QDir().mkdir(path);
	}

	const auto fn(path + "/position.data");
	QFile file(fn);

	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		qDebug() << fileName << " not open";
		return;
	}

	QVector<Position> toFile;
	for (auto position : positions) 
		toFile.append(*position);

	QDataStream streamFile(&file);
	streamFile << toFile;

	file.close();
}

void PositionController::readFromFile()
{
	if (!profile.size()) return;

	if (!QDir("profile").exists())
	{
		QDir().mkdir("profile");
	}

	const auto path("profile/" + profile);
	if (!QDir(path).exists())
	{
		QDir().mkdir(path);
	}

	const auto fn(path + "/position.data");
	QFile file(fn);

	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << fileName << " not open";
		return;
	}

	QVector<Position> fromFile;
	QDataStream streamFile(&file);
	streamFile >> fromFile;
	file.close();

	//todo: memory leak
	positions.clear();
	for (const auto &position : fromFile)
	{
		auto ptr = new Position(position);
		if (trade.isAvailable())
		{
			ptr->setTradeApi(trade);
		}
		
		QObject::connect(ptr, &Position::requestToSave, [this]()
		{
			writeToFile();
		});

		QObject::connect(ptr, &Position::removing, this, &PositionController::remove);

		positions.push_back(ptr);
	}
}

bool PositionController::isReady() const
{
	return trade.isAvailable();
}

Position * PositionController::getPosition(const QByteArray &hash) const
{
	Position *result = nullptr;

	for (const auto position : positions)
	{
		const auto checkedHash = position->getHash();
		if (checkedHash == hash)
		{
			result = position;
		}
	}

	return result;
}