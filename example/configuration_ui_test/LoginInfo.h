#pragma once

#include "common/config/WSettingExchangeable.h"
#include "common/config/WSettingConfigArray.h"
#include <QString>
#include <QImage>

class WSettingExchanger;

class LoginInfo : public WSettingExchangeable
{
public:
	LoginInfo();
    LoginInfo(const QString& _userName,
			  const QString& _password);
    virtual ~LoginInfo();
	virtual void exchange(WSettingExchanger& e);
	QString userName;
	QString password;
	QImage gravatar;
};

typedef WSettingConfigArray<LoginInfo> LoginInfoList;



