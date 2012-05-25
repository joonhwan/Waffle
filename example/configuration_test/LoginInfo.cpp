#include "LoginInfo.h"
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
void LoginInfo::exchange(WSettingExchanger& s)
{
	s.handle(userName, "username", "DefaultUserName");
	s.handle(password, "password", "dammPassword");
}

