#pragma once

#include <QSettings>

class QString;

namespace Wf {

bool hasKeyStartsWith(const QSettings& settings, const QString& s);
bool hasKeyStartsWith(const QSettings::SettingsMap& map, const QString& s);

} // namespace Wf
