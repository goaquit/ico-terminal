#pragma once
#include <QString>

namespace SqlRequest
{


const QString exchangeList(
	"SELECT id, name FROM exchange ORDER BY name ASC;"
);

const QString exchangeAvailableList(
	R"(select distinct e.id, e.name 
		from trades_last_index as t
		left join exchange as e on e.id = t.exchange
		order by id;)"
);

const QString pairAvailableList(
	"select id,title from get_available_pair();"
);

const QString getPairExchange(
	"select id from getPairsStrExchange(%1);"
);


}