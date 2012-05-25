#pragma once

#include "db/WSqlHeader.h"
#include <QVariant>

class WSqlQueryBinder
{
public:
	WSqlQueryBinder(QSqlQuery& query);
	virtual ~WSqlQueryBinder();
	WSqlQueryBinder& bind(const QString& field,
						  const QVariant& value);
protected:
	QSqlQuery& m_query;
};
