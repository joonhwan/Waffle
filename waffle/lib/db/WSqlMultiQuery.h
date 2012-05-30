#pragma once

#include "db/WSqlHeader.h"
#include <QStringList>

class WSqlMultiQuery
{
public:
    WSqlMultiQuery(const QStringList& queryStringList, QSqlDatabase db = QSqlDatabase());
	WSqlMultiQuery(QSqlDatabase db = QSqlDatabase());
    virtual ~WSqlMultiQuery();
	bool exec(const QStringList& queryStringList);
	const QSqlQuery& query() const {
		return m_query;
	}
	QSqlQuery& query() {
		return m_query;
	}
	bool isActive() const {
		if (m_queryStringList.empty()) {
			return true;
		} else {
			return m_query.isActive();
		}
	}
	QSqlError lastError() {
		return m_query.lastError();
	}
protected:
	QSqlQuery m_query;
	QStringList m_queryStringList;
};
