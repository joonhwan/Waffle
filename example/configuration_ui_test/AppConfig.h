#pragma once

#include "AppConfigEnums.h"
#include "SubConfig.h"

class WSettingExchanger;

class AppConfig : public WSettingExchangeable
{
public:
    AppConfig();
    virtual ~AppConfig();
	virtual void exchange(WSettingExchanger& e);

	bool useProxy;
	QString customer;
	QString serverName;
	SubConfig subConfig;
	AppConfigEnums::Priority priority;
};
