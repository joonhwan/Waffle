#include "WSettingExchangeable.h"
#include "WSettingAccessor.h"
#include "WSettingAccessorToQSettings.h"
#include "WSettingExchanger.h"
#include <QSettings>
#include <QString>

//virtual
void WSettingExchangeable::loadFrom(QSettings& s, const QString& key)
{
	s.beginGroup(key);
	WSettingAccessorToQSettings accessor(s, WSettingAccessor::Read);
	WSettingExchanger exchanger(accessor);
	exchange(exchanger);
	s.endGroup();
}

//virtual
void WSettingExchangeable::saveTo(QSettings& s, const QString& key)
{
	s.beginGroup(key);
	WSettingAccessorToQSettings accessor(s, WSettingAccessor::Write);
	WSettingExchanger exchanger(accessor);
	exchange(exchanger);
	s.endGroup();
}

//virtual
void WSettingExchangeable::loadFrom(WSettingAccessor& s, const QString& key)
{
	Q_ASSERT(s.isReading());
	s.beginGroup(key);
	WSettingExchanger exchanger(s);
	exchange(exchanger);
	s.endGroup();
}

//virtual
void WSettingExchangeable::saveTo(WSettingAccessor& s, const QString& key)
{
	Q_ASSERT(!s.isReading());
	s.beginGroup(key);
	WSettingExchanger exchanger(s);
	exchange(exchanger);
	s.endGroup();
}
