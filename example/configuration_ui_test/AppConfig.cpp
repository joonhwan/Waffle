#include "AppConfig.h"

namespace {

LoginInfoList defaultConfig = LoginInfoList();

}

AppConfig::AppConfig()
{
	priority = AppConfigEnums::High;
}

//virtual
AppConfig::~AppConfig()
{
}

//virtual
void AppConfig::exchange(WSettingExchanger& e)
{
	e.handle(useProxy, "useProxy", true);
	e.handle(customer, "customer", "SHARP");
	e.handle(serverName, "servername", "www.joonhwan.org");
	e.handle(logins, "logins", LoginInfoList());
	e.handle(priority, "priority", priority);
}
