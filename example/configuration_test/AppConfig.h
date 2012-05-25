#pragma once

#include "LoginInfo.h"

class WSettingExchanger;

class AppConfig : public WSettingExchangeable
{
public:
    AppConfig();
    virtual ~AppConfig();
	virtual void exchange(WSettingExchanger& e);

	QString customer;
	QString serverName;
	LoginInfoList logins;
};
