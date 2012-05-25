// -*- coding : utf-8-le-dos -*-
#include "common/config/WLuaSettings.h"
#include "gui/WSettingsEditor.h"
#include "gui/WSettingExchangeableEditor.h"
#include "AppConfig.h"
#include "LoginInfo.h"
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
		// part of settings can be handled like this
		LoginInfoList localLoginInfo;
		localLoginInfo.loadFrom(settings, "AppConfig/logins");

		// localLoginInfo << LoginInfo("addedUser", "asdoifjawer");
		// localLoginInfo.saveTo(settings, "AppConfig/logins");
	}

	WSettingsEditor editor(settings);
	editor.setWindowTitle(QObject::tr("WSettingsEditor"));
	editor.show();

	// QSettings googleSettings(QSettings::SystemScope, "Google", "Update");
	// WSettingsEditor googleSettingsEditor(googleSettings);
	// googleSettingsEditor.show();
	AppConfig _appConfig;
	_appConfig.loadFrom(settings, "AppConfig");
	QWidget customEditor;
	{
		QVBoxLayout* layout = new QVBoxLayout;
		WSettingExchangeableEditor* inEditor
			= new WSettingExchangeableEditor(_appConfig, QString::fromUtf16((const ushort*)L"설정にほんごfåtölj"),
											 WSettingExchangeableEditor::Button);
		QPushButton* saveButton = new QPushButton("&Save");
		QPushButton* restoreButton = new QPushButton("&Restore");
		customEditor.connect(saveButton, SIGNAL(clicked()),
							 inEditor, SLOT(save()));
		customEditor.connect(restoreButton, SIGNAL(clicked()),
							 inEditor, SLOT(restore()));
		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addStretch(1);
		buttonLayout->addWidget(saveButton);
		buttonLayout->addWidget(restoreButton);
		layout->addWidget(inEditor);
		layout->addLayout(buttonLayout);
		customEditor.setLayout(layout);
	}
	customEditor.setWindowTitle(QObject::tr("Custom Editor"));
	customEditor.show();

	// eeditor.save();
	// eeditor.restore();
	return app.exec();
};
