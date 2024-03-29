#pragma once

#include "common/config/WSettingExchangeTraits.h"

class QSettings;
class QString;
class WSettingExchanger;
class WSettingAccessor;

class WSettingExchangeable
{
public:
	typedef WGroupSettingTag ExchangeCategory;
	virtual void exchange(WSettingExchanger& e) = 0;
	virtual void loadFrom(QSettings& s, const QString& key);
	virtual void saveTo(QSettings& s, const QString& key);
	virtual void loadFrom(WSettingAccessor& s, const QString& key);
	virtual void saveTo(WSettingAccessor& s, const QString& key);
};
