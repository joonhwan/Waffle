#pragma once

#include "db/WSqlHeader.h"
#include <QStringList>
#include <QVariantMap>

class WSqlSimpleInsertQueryBuilder
{
public:
	WSqlSimpleInsertQueryBuilder(const QString& tableName = QString(),
						   QSqlDatabase db = QSqlDatabase());
    virtual ~WSqlSimpleInsertQueryBuilder();
	WSqlSimpleInsertQueryBuilder& useQuote(bool use=true);
	WSqlSimpleInsertQueryBuilder& keyValue(const QString& fieldName, const QVariant& value);
	QSqlQuery query() const;
protected:
	bool m_useQuote;
	QString m_tableName;
	QVariantMap m_fields;
	QSqlDatabase m_db;
};
