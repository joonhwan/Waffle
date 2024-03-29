#include "Protocol.h"
#include "TripServer.h"
#include <QApplication>
#include <QPushButton>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("trip-planner-server");

	initializeProtocol();

	TripServer server;
	if (!server.listen(QHostAddress::Any, TRIP_SERVER_PORT)) {
		return 1;
	}

	QPushButton* quitButton = new QPushButton("&Quit");
	QObject::connect(quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
	quitButton->setWindowTitle(QCoreApplication::applicationName());
	quitButton->show();

	return app.exec();
}
