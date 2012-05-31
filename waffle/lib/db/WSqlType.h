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

