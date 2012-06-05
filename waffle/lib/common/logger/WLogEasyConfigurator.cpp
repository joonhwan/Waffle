#include "WLogEasyConfigurator.h"
#include "WLogger.h"
#include "WLogOutput.h"
#include "WLogFileOutput.h"
#include <QCoreApplication>
#include <QDir>
#include <QString>

WLogEasyConfigurator::WLogEasyConfigurator(Wf::LogLevel defaultLogLevel)
	: m_defaultLogFilePath("default.log")
	, m_defaultLogLevel(defaultLogLevel)
{
	m_defaultLogFilePath = QDir(QCoreApplication::applicationDirPath()).filePath("log.txt");
	initializeLog();
}

//virtual
WLogEasyConfigurator::~WLogEasyConfigurator()
{
}

void WLogEasyConfigurator::initializeLog(void)
{
	// init the logging mechanism
	WLogger& logger = WLogger::instance();

	// add destinations for "root logger"
	// WLogOutputPtr debugOut(new WLogDebugOutput());
	// logger.addLogOutput(debugOut);

	WLogFileOutput* fileOutP = new WLogFileOutput(m_defaultLogFilePath);
	WLogOutputPtr fileOut(fileOutP);
	logger.addLogOutput(fileOut);
	logger.setLoggingLevel(m_defaultLogLevel);
}
