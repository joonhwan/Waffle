#include "wlogevent.h"
#include <QThread>
#include <cassert>

using namespace Wf;

namespace {

static const char TraceString[] = "TRACE";
static const char DebugString[] = "DEBUG";
static const char InfoString[]  = "INFO";
static const char WarnString[]  = "WARN";
static const char ErrorString[] = "ERROR";
static const char FatalString[] = "FATAL";

const char* LevelToText(LogLevel theLevel)
{
   switch( theLevel )
   {
   case TraceLevel:
      return TraceString;
   case DebugLevel:
      return DebugString;
   case InfoLevel:
      return InfoString;
   case WarnLevel:
      return WarnString;
   case ErrorLevel:
      return ErrorString;
   case FatalLevel:
      return FatalString;
   default:
      {
         assert(!"bad log level");
         return InfoString;
      }
   }
}

}


WLogEvent::WLogEvent()
{
}

WLogEvent::WLogEvent(const QString& _target,
					 Wf::LogLevel  _level,
					 const QString& _log)
	: now(QDateTime::currentDateTime())
	, target(_target)
	, level(_level)
	, log(_log)
{
	levelName = QString::fromAscii(LevelToText(level));
	// #define USE_REAL_THREAD_NAME
#ifdef USE_REAL_THREAD_NAME
	if(QThread::currentThread()) {
		threadName = QThread::currentThread()->objectName();
	}
#endif
	if(threadName.isEmpty()) {
		threadName = QString("%1").arg((long)QThread::currentThreadId(),8);
	}
}

//virtual
WLogEvent::~WLogEvent()
{
}

QString WLogEvent::logMessage(void) const
{
	return QString("%1 %2 [%3][%4] %5")
		.arg(now.toString("yy/MM/dd-hh:mm:ss.zzz"))
		.arg(levelName, 5)
		.arg(target, 10)
		.arg(threadName, 8)
		.arg(log)
		;
}

