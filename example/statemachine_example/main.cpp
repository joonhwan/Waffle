#include "testerwindow.h"
#include "common/logger/wlogger.h"
#include "common/logger/wlogdebugoutput.h"
#include "common/logger/wlogfileoutput.h"
#include <QDir>
#include <QtGui/QApplication>
#include <QTimer>

void initializeLog(void)
{
	// init the logging mechanism
	WLogger& logger = WLogger::instance();

	// add destinations for "root logger"
	// WLogOutputPtr debugOut(new WLogDebugOutput());
	// logger.addLogOutput(debugOut);
	const QString sLogPath(QDir(QCoreApplication::applicationDirPath()).filePath("log.txt"));
	WLogFileOutput* fileOutP = new WLogFileOutput(sLogPath);
	WLogOutputPtr fileOut(fileOutP);
	logger.addLogOutput(fileOut);
	logger.setLoggingLevel(Wf::TraceLevel);
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	initializeLog();

	TesterWindow window;
	window.show();
	// QTimer::singleShot(1000, &a, SLOT(quit()));
	return a.exec();
}
