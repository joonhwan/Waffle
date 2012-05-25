#pragma once

#include "AppConfigEnums.h"
#include "LoginInfo.h"

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
	LoginInfoList logins;
	AppConfigEnums::Priority priority;
};
