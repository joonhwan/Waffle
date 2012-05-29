#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>

namespace Wf {

enum DbType {
	DbUnknown = 0,

	// fixed length
	DbSmallInt,
	DbInt,
	DbBigInt,
	DbFloat,
	DbDouble,
	DbDate,
	// variable length(length info needed)
	DbVarString,
	DbVarByte,
	DbFixString,
	DbFixByte,

	DbTypeCount,
};

enum DbFieldAttribute {
	DbNotNull = 0x1,
	DbPrimaryKey = 0x2,
	DbForeignKey = 0x4,
	DbAutoIncrement = 0x8,
	DbHasDefault = 0x10,
};

}


struct FieldDefinition
{
	FieldDefinition()
		: dbType(Wf::DbUnknown)
		, attribute(0)
		, length(0)
	{
	}
	bool isNormalField() const {
		return
			!isPrimaryKey() && !isForeignKey();
	}
	bool isPrimaryKey() const {
		return (attribute & Wf::DbPrimaryKey) == Wf::DbPrimaryKey;
	}
	bool isForeignKey() const {
		return (attribute & Wf::DbForeignKey) == Wf::DbForeignKey;
	}
	bool hasDefaultValue() const {
		return (attribute & Wf::DbHasDefault) == Wf::DbHasDefault;
	}
	QString fieldName;
	QString dataType;
	Wf::DbType dbType;
	int attribute;
	int length;
	QString defaultValue;
	QVariantMap properties;
};

typedef QList<FieldDefinition> FieldDefinitionList;
