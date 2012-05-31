#pragma once

#include "db/WSqlHeader.h"
#include "db/WSqlTableDefinition.h"
#include "flamerobin/flamerobin.h"
#include <QString>

class WSqlFirebirdTableBuilder
{
public:
    WSqlFirebirdTableBuilder(const WSqlTableDefinition& tableDefinition,
							 QSqlDatabase db = QSqlDatabase::database());
    virtual ~WSqlFirebirdTableBuilder();
	bool tryGetUpdateQueryString(QStringList& updateQuery);
	QStringList createQueryString();
	QString lastError() {
		return m_lastError;
	}
	void test();
protected:
	QSqlDatabase m_db; // Qt's database handle
	fr::DatabasePtr m_frDb; // flamerobin's database handle
	QString m_lastError;
	WSqlTableDefinition m_tableDefinition;

	WSqlFieldDefinitionList getFieldDefinition(fr::TablePtr& table);
	QString normalPrimaryKeyConstraintName(const QString& tableName) const;
	QString normalForeignKeyConstraintName(const QString& fieldName) const;
	QString normalGeneratorNameOf(const QString& tableName, const QString& fieldName) const;
	QString normalTriggerNameForGenerator(const QString& tableName, const QString& fieldName) const;
	bool valueNeedQuoteFor(const QString& dataType) const;

	QStringList getTableSql(const WSqlFieldDefinitionList& fdl, bool useAlterStatement=false) const;
};
