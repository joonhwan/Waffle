#include "LoginInfo.h"
#include "common/config/WSettingAccessorToQSettings.h"
#include <QSettings>

LoginInfo::LoginInfo()
{
}

LoginInfo::LoginInfo(const QString& _userName,
					 const QString& _password)
	: userName(_userName)
	, password(_password)
{
	// gravatar.load("c:/temp/castle.jpg");
}

//virtual
LoginInfo::~LoginInfo()
{
}

// virtual
void LoginInfo::exchange(WSettingExchanger& e)
{
	e.handle(userName, "username", "DefaultUserName");
	e.handle(password, "password", "dammPassword");
}

