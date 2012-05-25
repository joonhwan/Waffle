#pragma once

#include <QString>
#include <QVariant>
#include <QMap>

class WSettingsEditInfo
{
	struct Info
	{
		QString guiName;
		QString guiDescription;
		QVariant defaultValue;
	};
public:
	QString guiName(const QString& settingKey) const;
	QString guiToolTip(const QString& settingKey) const;
protected:
	QMap<QString, Info> m_infos;
};
