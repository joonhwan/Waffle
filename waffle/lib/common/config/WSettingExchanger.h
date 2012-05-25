#pragma once

#include "common/config/WSettingExchangeTraits.h"
#include "common/config/WSettingAccessor.h"
#include <QMetaEnum>
#include <QStringList>

class WSettingExchanger
{
public:
	WSettingExchanger(WSettingAccessor& accessor)
		: m_accessor(accessor)
	{
	}
	bool isLoading() const {
		return m_accessor.isReading();
	}
	WSettingAccessor& accessor() {
		return m_accessor;
	}
	const WSettingAccessor& accessor() const {
		return m_accessor;
	}
	template<typename T, typename R>
	WSettingExchanger& handle(T& value, const QString& key, const R& defaultValue)
	{
		return handle(value, key, defaultValue, WExchangeTrait<T>::ExchangeCategory());
	}
protected:
	WSettingAccessor& m_accessor;

	template<typename T, typename R>
	WSettingExchanger& handle(T& value, const QString& key, const R& defaultValue, WSingleValueSettingTag)
	{
		if (isLoading()) {
			value = m_accessor.value(key, defaultValue).value<T>();
		} else {
			m_accessor.setValue(key, value);
		}
		return *this;
	}
	template<typename T, typename R>
	WSettingExchanger& handle(T& value, const QString& key, const R& defaultValue, WEnumSettingTag)
	{
		const char* enumName = WEnumMetaObjectOf<T>::enumName();
		int enumIndex = WEnumMetaObjectOf<T>::Type::staticMetaObject.indexOfEnumerator(enumName);
		Q_ASSERT(enumIndex >= 0);
		if (enumIndex >= 0) {
			QMetaEnum metaEnum = WEnumMetaObjectOf<T>::Type::staticMetaObject.enumerator(enumIndex);
			QStringList enumNames;
			for (int i = 0; i < metaEnum.keyCount(); ++i) {
				enumNames << QLatin1String(metaEnum.key(i));
			}
			accessor().setEnumListHint(key, enumNames);

			if (isLoading()) {
				QString valueInString = m_accessor.value(key, metaEnum.valueToKey(defaultValue)).value<QString>();
				value = (T)metaEnum.keyToValue(valueInString.toAscii());
			} else {
				m_accessor.setValue(key, metaEnum.valueToKey(value));
			}
		} else {
			// handle it like integer.
			if (isLoading()) {
				value = (T)m_accessor.value(key, (int)defaultValue).value<int>();
			} else {
				m_accessor.setValue(key, (int)value);
			}
		}
		return *this;
	}
	template<typename T, typename R>
	WSettingExchanger& handle(T& value, const QString& key, const R& defaultValue, WArraySettingTag)
	{
		value.exchange(*this, key, defaultValue);
		return *this;
	}
};
