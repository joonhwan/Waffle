#pragma once

#include <QList>
#include <QString>

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

}

struct FieldDefinition
{
	FieldDefinition()
		: dbType(Wf::DbUnknown)
		, length(0)
	{
	}
	QString fieldName;
	Wf::DbType dbType;
	int optional;
	// optional
	int length;
};

typedef QList<FieldDefinition> FieldDefinitionList;
