#pragma once

#include <QMutex>
#include <QSettings>

class QIODevice;

class WLuaSettings : public QSettings
{
    Q_OBJECT
public:
	WLuaSettings(const QString& fileName, QObject* parent=0);
    virtual ~WLuaSettings();
	static bool readLuaFileCB(QIODevice &device, QSettings::SettingsMap &map);
	static bool writeLuaFileCB(QIODevice &device, const QSettings::SettingsMap &map);
private:
	QString m_fileName;
	static Format s_format;
	static QMutex s_lock;
	friend class LuaSettingsLocker;
};

Q_DECLARE_METATYPE(WLuaSettings*);
