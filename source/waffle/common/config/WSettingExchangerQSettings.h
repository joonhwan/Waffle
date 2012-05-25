#pragma once

#include "common/cnofig/WSettingExchanger.h"
#include <QSettings>

class WSettingExchangerQSettings
{
public:
    WSettingExchangerQSettings(QSettings& s, bool isLoading)
		: WSettingExchagner(isLoading)
		, m_settings(s)
	{
	}
};
