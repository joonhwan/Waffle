#pragma once

#include "common/config/WSettingExchangeable.h"
#include "LoginInfo.h"

class SubConfig : public WSettingExchangeable
{
public:
	SubConfig();
	virtual ~SubConfig();
	virtual void exchange(WSettingExchanger& e);

	int comPort;
	int baudRate;
	LoginInfoList logins;
};
