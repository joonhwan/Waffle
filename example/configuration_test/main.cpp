#include <QApplication>
#include <QDebug>
#include <QSettings>
#include "common/config/WLuaSettings.h"
#include "AppConfig.h"
#include "LoginInfo.h"

using namespace Wf;

int main(int argc, char** argv)
{
	qDebug() << "begin testing custom settings....";

	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("customsettings");

	{
		WLuaSettings settings("c:/temp/customsettings.lua");
		// QSettings settings;

		{
			AppConfig appConfig;
			appConfig.loadFrom(settings, "AppConfig");

			LoginInfoList& logins = appConfig.logins;
			if (logins.empty()) {
				logins.clear();
				logins << LoginInfo("joonhwan", "winkler7")
					   << LoginInfo("seoyeon", "sally")
					   << LoginInfo("eonseo", "maja");
			}

			appConfig.customer = appConfig.customer + QLatin1String("P");
			appConfig.saveTo(settings, "AppConfig");
		}

		{
			// LoginInfoList anotherInfoList;
			// anotherInfoList.loadFrom(settings, "AppConfig/logins");
			AppConfig appConfig;
			appConfig.loadFrom(settings, "AppConfig");
			LoginInfoList& anotherInfoList = appConfig.logins;

			anotherInfoList << LoginInfo("addedUser", "asdoifjawer");
			anotherInfoList.saveTo(settings, "AppConfig/logins");
		}
	}
	return 0;
	// MainWindow w;
	// w.show();
	// return app.exec();
};

