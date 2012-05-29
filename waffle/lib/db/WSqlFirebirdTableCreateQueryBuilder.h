#pragma once

#include "db/WSqlHeader.h"
#include "db/WSqlType.h"
#include <QStringList>

// Table creation query builder for Firebird
class WSqlFirebirdTableCreateQueryBuilder
{
public:
	WSqlFirebirdTableCreateQueryBuilder(const QString& tableName,
										QSqlDatabase db = QSqlDatabase());
	virtual ~WSqlFirebirdTableCreateQueryBuilder();
	void setUseQuotedFieldName(bool quote=false);
	// calling order decide field's position
	// WSqlFirebirdTableCreateQueryBuilder& field(const QString& fieldName,
	// 										   const QString& dbType,
	// 										   bool attribute=false);
	// WSqlFirebirdTableCreateQueryBuilder& optionalField(const QString& fieldName,
	// 												   const QString& dbType) {
	// 	return field(fieldName, dbType, true);
	// }
	WSqlFirebirdTableCreateQueryBuilder& field(const QString& fieldName,
											   Wf::DbType dbType,
											   Wf::DbFieldAttribute attribute=Wf::DbNotNull);
	WSqlFirebirdTableCreateQueryBuilder& field(const QString& fieldName,
											   Wf::DbType dbType,
											   int length,
											   Wf::DbFieldAttribute attribute=Wf::DbNotNull);
	WSqlFirebirdTableCreateQueryBuilder& field(FieldDefinition& fieldDefinition);

	WSqlFirebirdTableCreateQueryBuilder& serialPrimaryKey(const QString& fieldName);

	WSqlFirebirdTableCreateQueryBuilder& foreignSerialKey(const QString& fieldName,
														  const QString& foreignTable,
														  const QString& foreignTableKey);

	QSqlQuery query() const;
protected:
	QSqlDatabase m_db;
	QString m_tableName;
	bool m_useQuotedFieldName;
	FieldDefinitionList m_fieldDefinitionList;
	QStringList m_ddl;
	QStringList m_constraints;
	QStringList m_psql;

	QString normalize(const QString& fieldName) const;
	QString fieldDefinitionToSql(const FieldDefinition& fd) const;
};
