#include "MainWindow.h"
#include <QApplication>

// #if defined(_DEBUG) && defined(WIN32)
// #define _CRTDBG_MAP_ALLOC
// #include <stdlib.h>
// #include <crtdbg.h>
// class MemLeakDetector
// {
// public:
// 	MemLeakDetector()
// 	{
// 		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
// 	}
// 	~MemLeakDetector()
// 	{
// 	}
// } detector;
// #endif

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("packet_test");

	MainWindow w;
	w.show();

	return app.exec();
}
