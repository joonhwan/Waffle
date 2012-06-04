#include "MainWindow.h"
#include "Protocol.h"
#include <QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("trip-planner");

	initializeProtocol();

	MainWindow w;
	w.show();

	return app.exec();
}
