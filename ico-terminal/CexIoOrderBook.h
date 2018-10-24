#pragma once

#include "AbstractOrderBookLoader.h"

namespace TerminalCore
{
	class CexIoOrderBook : public AbstractOrderBookLoader
	{
	public:
		QList<Order> loadAsks() override;
		QList<Order> loadBids() override;
	};
}