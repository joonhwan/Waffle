#include "WSettingEditInfo.h"

QString WSettingExchangeableEditInfo::guiName(const QString& settingKey) const
{
	QString v = settingKey;
	if (m_infos.contains(settingKey)) {
		v = m_infos[settingKey].guiName;
	} else {
		QString parentKey;
		split(settingKey, parentKey, v);
	}
	return v;
}

QString WSettingExchangeableEditInfo::guiToolTip(const QString& settingKey) const
{
	QString v = settingKey;
	if (m_infos.contains(settingKey)) {
		v = m_infos[settingKey].guiName;
	}
	return v;
}
