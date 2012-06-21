#include "MainWindow.h"
#include "common/global/WMemoryLeakDetect.h"
#include <QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("vector_draw_example");

	MainWindow w;
	w.show();
	w.raise();
	return app.exec();
}
