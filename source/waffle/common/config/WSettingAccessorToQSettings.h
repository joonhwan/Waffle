#pragma once

#include "common/config/WSettingAccessor.h"
#include "common/config/WSettingsHelper.h"
#include <QSettings>

// general accessor
class WSettingAccessorToQSettings : public WSettingAccessor
{
public:
	WSettingAccessorToQSettings(QSettings& settings, Type type)
		: WSettingAccessor(type)
		, m_settings(settings)
	{
	}
	virtual bool hasKeyStartsWith(const QString& key)
	{
		return Wf::hasKeyStartsWith(m_settings, key);
	}
	virtual void beginGroup(const QString& key)
	{
		m_settings.beginGroup(key);
	}
	virtual void endGroup()
	{
		m_settings.endGroup();
	}
	virtual int beginArray(const QString& key)
	{
		if (m_type==Read) {
			return m_settings.beginReadArray(key);
		} else {
			m_settings.beginWriteArray(key);
			return 0;
		}
	}
	virtual void beginArrayIndex(int index)
	{
		m_settings.setArrayIndex(index);
	}
	virtual void endArrayIndex()
	{
		// nothing to do
	}
	virtual void endArray()
	{
		m_settings.endArray();
	}
	virtual QVariant value(const QString& key, const QVariant& defaultValue)
	{
		return m_settings.value(key, defaultValue);
	}
	virtual void setValue(const QString& key, const QVariant& value)
	{
		m_settings.setValue(key, value);
	}
protected:
	QSettings& m_settings;
};
