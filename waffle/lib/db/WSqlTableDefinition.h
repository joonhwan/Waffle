#pragma once

#include "db/WSqlType.h"
#include "db/WSqlFieldDefinition.h"

class WSqlTableDefinition
{
public:
	WSqlTableDefinition(const QString& tableName = QString());
	WSqlTableDefinition& serialPrimaryKey(const QString& fieldName);
	WSqlTableDefinition& field(const QString& fieldName,
							   const QString& displayName,
							   const QString& dataType,
							   int attribute,
							   const QString& defaultValue = QString());
	WSqlTableDefinition& foreignSerialKey(const QString& fieldName,
										  const QString& foreignTable,
										  const QString& foreignTableKey);
	WSqlTableDefinition& field(const WSqlFieldDefinition& definition);

	const QString& tableName() const {
		return m_tableName;
	}
	WSqlFieldDefinitionList fields() const {
		return m_fieldDefinitionList;
	}
	WSqlFieldDefinition operator[](const QString& fieldName) const;
protected:
	QString m_tableName;
	WSqlFieldDefinitionList m_fieldDefinitionList;
};
