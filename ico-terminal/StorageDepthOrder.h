#pragma once

#include <QObject>
#include "Client.h"
#include "DepthOrder.h"
#include <QString>

#include <map>

using namespace Entity;

namespace TerminalCore
{


class StorageDepthOrder : public QObject
{
	Q_OBJECT

public:
	StorageDepthOrder(QObject *parent);
	~StorageDepthOrder();

	void run();

private:
	Loader::Client client;

	std::map<QString, DepthOrder> depthOrders;
};



}