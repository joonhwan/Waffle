#include "WSqlTableDefinition.h"

WSqlTableDefinition::WSqlTableDefinition(const QString& tableName)
	: m_tableName(tableName)
{
}

WSqlTableDefinition&
WSqlTableDefinition::serialPrimaryKey(const QString& fieldName)
{
	WSqlFieldDefinition fd(fieldName,
						   "bigint",
						   Wf::DbNotNull | Wf::DbPrimaryKey | Wf::DbAutoIncrement);
	return field(fd);
}

WSqlTableDefinition&
WSqlTableDefinition::field(const QString& fieldName,
						   const QString& displayName,
						   const QString& dataType,
						   int attribute,
						   const QString& defaultValue)
{
	WSqlFieldDefinition fd(fieldName,
						   dataType,
						   attribute);
	fd.setDisplayName(displayName);
	if (!defaultValue.isEmpty()) {
		fd.setDefaultValue(defaultValue);
		fd.addAttribute(Wf::DbHasDefault);
	}
	return field(fd);
}

WSqlTableDefinition&
WSqlTableDefinition::foreignSerialKey(const QString& fieldName,
									  const QString& foreignTable,
									  const QString& foreignTableKey)
{
	WSqlFieldDefinition fd(fieldName,
						   "bigint",
						   Wf::DbNotNull | Wf::DbForeignKey);
	fd.setProperty("foreignTable",foreignTable);
	fd.setProperty("foreignTableKey", foreignTableKey);
	return field(fd);
}

WSqlTableDefinition&
WSqlTableDefinition::field(const WSqlFieldDefinition& definition)
{
	WSqlFieldDefinition fd = definition;
	fd.setTableName(m_tableName);
	m_fieldDefinitionList << fd;
	return *this;
}

WSqlFieldDefinition
WSqlTableDefinition::operator[](const QString& fieldName) const
{
	WSqlFieldDefinition v;
	foreach(const WSqlFieldDefinition& fd, m_fieldDefinitionList) {
		if (fd.fieldName() == fieldName) {
			v = fd;
		}
	}
	return v;
}
