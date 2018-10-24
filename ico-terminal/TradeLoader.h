#include "AbstractTradeLoader.h"
#include <QSqlDataBase>
namespace Terminal
{
	class TradeLoader : public AbstractTradeLoader
	{
		QSqlDatabase *db = nullptr;
	public:

		QList<TradePrice>  loadData(
			time_t start,
			time_t end,
			time_t timeFrame
		) override;
	};
}