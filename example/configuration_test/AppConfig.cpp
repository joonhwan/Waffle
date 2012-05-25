#include "AppConfig.h"

namespace {

LoginInfoList defaultConfig = LoginInfoList();

}

AppConfig::AppConfig()
{
}

//virtual
AppConfig::~AppConfig()
{
}

//virtual
void AppConfig::exchange(WSettingExchanger& e)
{
	e.handle(customer, "customer", "SHARP");
	e.handle(serverName, "servername", "www.joonhwan.org");
	e.handle(logins, "logins", LoginInfoList());
}
