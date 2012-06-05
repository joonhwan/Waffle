#include "WMainWindow.h"
#include <QSettings>

WMainWindow::WMainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_showedOnce(false)
{
}

//virtual
WMainWindow::~WMainWindow()
{
}

//virtual
void WMainWindow::showEvent(QShowEvent* e)
{
	QMainWindow::showEvent(e);
	if (!m_showedOnce) {
		restoreWindowState();
		m_showedOnce = true;
	}
}

//virtual
void WMainWindow::closeEvent(QCloseEvent* e)
{
	QSettings settings;
	settings.beginGroup(settingName());
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.endGroup();

	QMainWindow::closeEvent(e);
}

void WMainWindow::restoreWindowState()
{
	QSettings settings;
	settings.beginGroup(settingName());
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	settings.endGroup();
}

QString WMainWindow::settingName() const
{
	QString name = objectName();
	if (name.isEmpty()) {
		name = QString::fromLatin1("WMainWindow");
	}
	return name;
}

