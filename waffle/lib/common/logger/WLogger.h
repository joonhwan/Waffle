#ifndef LOG_H
#define LOG_H

#include "common/global/wnamespace.h"
#include "common/logger/wlogoutput.h"

#include <QDebug>
#include <QString>

class WLogEvent;
class WLoggerImpl; // d pointer

using Wf::LogLevel;

class WLogger
{
public:
	static const QString ROOT_KEY;
	static WLogger& instance()
	{
		static WLogger staticLog;
		return staticLog;
	}

	//! Adds a log message target. Don't add null targets.
	void addLogOutput(WLogOutputPtr& output);
	void addLogOutput(const QString& target, WLogOutputPtr& output);
	//! at a level < 'newLevel' will be ignored
	void setLoggingLevel(LogLevel newLevel);
	void setLoggingLevel(const QString& target, LogLevel newLevel);

	//! The default level is INFO
	LogLevel loggingLevel(void) const;
	LogLevel loggingLevel(const QString& target) const;

	//! The helper forwards the streaming to QDebug and builds the final
	//! log message.
	struct WLocationInfo
	{
		WLocationInfo(const QString& _sourceFilename,
					  const QString& _functionName,
					  int _lineNumber)
			: sourceFilename(_sourceFilename)
			, functionName(_functionName)
			, lineNumber(_lineNumber)
		{
		}
		const QString sourceFilename;
		const QString functionName;
		int lineNumber;
	};
	class Helper
	{
	public:
		explicit Helper(const QString& _target,
						LogLevel logLevel,
						// location info
						const QString& _sourceFilename,
						const QString& _functionName,
						int lineNumber)
			: level(logLevel)
			, qtDebug(&buffer)
			, target(_target)
			, locationInfo(_sourceFilename,
						   _functionName,
						   lineNumber)
		{
		}
		~Helper();
		QDebug& stream(void)
		{
			return qtDebug;
		}
	private:
		void writeToLog(void);

		LogLevel level;
		QString target;
		QString buffer;
		QDebug qtDebug;
		WLocationInfo locationInfo;
	};

private:
	WLogger();
	WLogger(const WLogger&);
	WLogger& operator=(const WLogger&);
	~WLogger();

	void write(const WLogEvent& event);

	WLoggerImpl* d;
};

// hijacked from log4cxx.
// see apache-log4cxx-x.y.z/src/main/include/log4cxx/spi/location/locationinfo.h
#if !defined(WLOG_LOCATION)
#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __WLOG_FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
#define __WLOG_FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__WLOG_FUNC__)
#define __WLOG_FUNC__ ""
#endif
#define WLOG_LOCATION __FILE__,__WLOG_FUNC__,__LINE__
#endif

#define WLOG(t,l)												   \
	if( WLogger::instance().loggingLevel(t) > Wf::l##Level ){} \
	else WLogger::Helper(t,Wf::l##Level,WLOG_LOCATION).stream()

#define WLOG_TRACE(t) WLOG(t,Trace)
#define WLOG_DEBUG(t) WLOG(t,Debug)
#define WLOG_INFO(t) WLOG(t,Info)
#define WLOG_WARN(t) WLOG(t,Warn)
#define WLOG_ERROR(t) WLOG(t,Error)
#define WLOG_FATAL(t) WLOG(t,Fatal)

#define WLOG_DEFAULT_TARGET ""
#define WLOG_TRACE_() WLOG_TRACE(WLOG_DEFAULT_TARGET)
#define WLOG_DEBUG_() WLOG_DEBUG(WLOG_DEFAULT_TARGET)
#define WLOG_INFO_() WLOG_INFO(WLOG_DEFAULT_TARGET)
#define WLOG_WARN_() WLOG_WARN(WLOG_DEFAULT_TARGET)
#define WLOG_ERROR_() WLOG_ERROR(WLOG_DEFAULT_TARGET)
#define WLOG_FATAL_() WLOG_FATAL(WLOG_DEFAULT_TARGET)

 
#endif // QSLOG_H
