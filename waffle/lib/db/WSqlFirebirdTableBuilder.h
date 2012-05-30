#pragma once

#include "db/WSqlHeader.h"
#include "db/WSqlType.h"
#include "flamerobin/flamerobin.h"
#include <QString>

class WSqlFirebirdTableBuilder
{
public:
    WSqlFirebirdTableBuilder(const QString& tableName,
							 QSqlDatabase db = QSqlDatabase::database());
    virtual ~WSqlFirebirdTableBuilder();

	WSqlFirebirdTableBuilder& serialPrimaryKey(const QString& fieldName);
	WSqlFirebirdTableBuilder& field(const QString& fieldName,
									const QString& dataType,
									int attribute,
									const QString& defaultValue = QString());
	WSqlFirebirdTableBuilder& foreignSerialKey(const QString& fieldName,
											   const QString& foreignTable,
											   const QString& foreignTableKey);
	WSqlFirebirdTableBuilder& field(const FieldDefinition& definition);
	bool tryGetUpdateQueryString(QStringList& updateQuery);
	QStringList createQueryString();
	QString lastError() {
		return m_lastError;
	}
	void test();
protected:
	QSqlDatabase m_db; // Qt's database handle
	fr::DatabasePtr m_frDb; // flamerobin's database handle
	QString m_tableName;
	QString m_lastError;
	FieldDefinitionList m_fieldDefinitionList;

	FieldDefinitionList getFieldDefinition(fr::TablePtr& table);
	QString normalPrimaryKeyConstraintName(const QString& tableName) const;
	QString normalForeignKeyConstraintName(const QString& fieldName) const;
	QString normalGeneratorNameOf(const QString& tableName, const QString& fieldName) const;
	QString normalTriggerNameForGenerator(const QString& tableName, const QString& fieldName) const;
	bool valueNeedQuoteFor(const QString& dataType) const;

	QStringList getTableSql(const FieldDefinitionList& fdl, bool useAlterStatement=false) const;
};
