// -*- coding : utf-8-le-dos -*-
#include "common/config/WLuaSettings.h"
#include "gui/WSettingsEditor.h"
#include "gui/WSettingExchangeableEditor.h"
#include "AppConfig.h"
#include "LoginInfo.h"
#include "CustomConfigEditor.h"
#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialog>

using namespace Wf;

int main(int argc, char** argv)
{
	qDebug() << "begin testing custom settings....";

	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("customsettings");

	// WLuaSettings settings("c:/temp/customsettings.lua");
	QSettings settings(QSettings::SystemScope, "Joonhwan", "configuration_ui_test");

	// whole configuration can be retrieved easily.
	{
		AppConfig appConfig;
		appConfig.loadFrom(settings, "AppConfig");

		LoginInfoList& logins = appConfig.subConfig.logins;
		if (logins.empty()) {
			logins.clear();
			logins << LoginInfo("joonhwan", "winkler7")
				   << LoginInfo("seoyeon", "sally")
				   << LoginInfo("eonseo", "maja");
		}

		appConfig.customer = appConfig.customer + QLatin1String("P");
		appConfig.saveTo(settings, "AppConfig");
	}

	// part of settings can be handled like this
	{
		LoginInfoList localLoginInfo;
		localLoginInfo.loadFrom(settings, "AppConfig/subConfig/logins");

		// localLoginInfo << LoginInfo("addedUser", "asdoifjawer");
		// localLoginInfo.saveTo(settings, "AppConfig/logins");
	}

	// general editor(configuration data oriented. no user defined type is needed!)
	WSettingsEditor editor(settings);
	editor.setWindowTitle(QObject::tr("WSettingsEditor"));
	editor.show();

	// custom editor(user defined type oriented)
	WSettingCustomConfigEditor<AppConfig> customEditor("AppConfig", settings);
	customEditor.setWindowTitle(QObject::tr("Custom Editor"));
	customEditor.show();

	// eeditor.save();
	// eeditor.restore();
	return app.exec();
};
