#include "WSettingsHelper.h"
#include <QStringList>

namespace Wf {

bool hasKeyStartsWith(const QSettings& settings, const QString& s)
{
	QStringList allKeys = settings.allKeys();
	bool found = false;
	foreach(const QString& key, allKeys) {
		if (key.startsWith(s)) {
			found = true;
			break;
		}
	}
	return found;
}

bool hasKeyStartsWith(const QSettings::SettingsMap& map, const QString& s)
{
	bool found = false;
	QSettings::SettingsMap::iterator it = map.begin();
	QSettings::SettingsMap::iterator itEnd = map.end();
	for (; it != itEnd; ++it) {
		if (it.key().startsWith(s)) {
			found = true;
			break;
		}
	}
	return found;
}

} //  namespace Wf
