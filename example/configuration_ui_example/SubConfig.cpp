#include "SubConfig.h"
#include "common/config/WSettingExchanger.h"

SubConfig::SubConfig()
{
}

//virtual
SubConfig::~SubConfig()
{
}

//virtual
void SubConfig::exchange(WSettingExchanger& e)
{
	e.handle(comPort, "comPort", 1);
	e.handle(baudRate, "baudRate", 19200);
	e.handle(logins, "logins", LoginInfoList());
}
