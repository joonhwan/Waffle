#pragma once

#include "db/WSqlHeader.h"
#include <iostream>

class WSqlTableLister
{
public:
	WSqlTableLister(const QString& tableName,
					int maxRecordToList=0,
					QSqlDatabase db = QSqlDatabase());
	QSqlQuery query() const;
	const QString& tableName() const {
		return m_tableName;
	}
	int maxList() const {
		return m_maxRecordToList;
	}
protected:
	QString m_tableName;
	int m_maxRecordToList;
	QSqlDatabase m_db;
};

std::ostream& operator << (std::ostream& os, WSqlTableLister& me);


