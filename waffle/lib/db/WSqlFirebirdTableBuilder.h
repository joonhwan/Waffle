#pragma once

#include "db/WSqlHeader.h"
#include "db/WSqlType.h"
#include "flamerobin/flamerobin.h"
#include <QString>

class QTextStream;

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
	bool tryGetUpdateQueryString(QString& updateQuery);
	QString createQueryString();

	void test();
protected:
	QSqlDatabase m_db; // Qt's database handle
	fr::DatabasePtr m_frDb; // flamerobin's database handle
	QString m_tableName;
	FieldDefinitionList m_fieldDefinitionList;

	FieldDefinitionList getFieldDefinition(fr::TablePtr& table, QTextStream& log);
	QString normalPrimaryKeyConstraintName();
	QString normalGeneratorNameOf(const QString& fieldName);
	QString normalTriggerNameForGenerator();
};
