#pragma once

struct WEnumInfo
{
	int index;
	QString name;
};

template<typename T>
QList<WEnumInfo> getEnums()
{
	QList<WEnumInfo> enumInfoList;

	const char* enumName = WEnumMetaObjectOf<T>::enumName();
	int enumIndex = WEnumMetaObjectOf<T>::Type::staticMetaObject.indexOfEnumerator(enumName);
	Q_ASSERT(enumIndex >= 0);
};

