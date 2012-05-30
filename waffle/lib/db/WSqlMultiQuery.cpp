#include "WSqlMultiQuery.h"

WSqlMultiQuery::WSqlMultiQuery(const QStringList& queryStringList,
							   QSqlDatabase db)
	: m_query(db)
{
	exec(queryStringList);
}

WSqlMultiQuery::WSqlMultiQuery(QSqlDatabase db)
	: m_query(db)
{
}

//virtual
WSqlMultiQuery::~WSqlMultiQuery()
{
}

bool WSqlMultiQuery::exec(const QStringList& queryStringList)
{
	m_queryStringList = queryStringList;

	bool failed = false;
	foreach(const QString& queryString, m_queryStringList) {
		if (!m_query.exec(queryString)) {
			failed = true;
			break;
		}
	}
	return failed;
}

