// -*- coding : utf-8-le-dos -*-

#include <QApplication>

using namespace Wf;

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("communication-test-server");

	return app.exec();
};
