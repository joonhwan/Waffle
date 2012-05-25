#include "wlogger.h"
#include "wlogoutput.h"

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QtGlobal>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

using namespace Wf;

inline
QString parentTargetName(const QString& target) {
	int pos = target.lastIndexOf(".");
	if(pos<0) {
		return "";
	} else {
		return target.left(pos);
	}
}

class WLogOutputList : public QList<WLogOutputPtr>
{
public:
	void writeToAll(const WLogEvent& event) {
		foreach(const WLogOutputPtr& dest, *this) {
			if( !dest ) {
				assert(!"null log target");
				continue;
			}
			dest->write(event);
		}
	}
};

struct WLogOutputInfo
{
	WLogOutputList outputList;
	bool explicitConfigured;
};

typedef QHash<QString, WLogOutputInfo> WLogOutputPool;

// not using Qt::ISODate because we need the milliseconds too
static const QString fmtDateTime("yyMMdd-hh:mm:ss.zzz");

struct LevelInfo
{
	LevelInfo(LogLevel _level = InfoLevel)
		: level(_level)
	{
	}
	LogLevel level;
	bool explicitlyConfigured;
};
typedef QHash<QString, LevelInfo> LevelPool;

class WLoggerImpl
{
public:
	WLoggerImpl()
	{
		perTargetLevelPool[WLogger::ROOT_KEY] = LevelInfo(InfoLevel);
	}
	void write(const WLogEvent& event)
	{
		// CHECKME correct order? deadlock?!
		QReadLocker locker(&lock);

		WLogOutputInfo* perTargetOutputInfo = 0;

		// try to find per-target target
		if(perTargetOutputListPool.size()) {
			QString key = event.target;
			while(!key.isEmpty())
			{
				WLogOutputPool::iterator it = perTargetOutputListPool.find(key);
				if(it!=perTargetOutputListPool.end()) {
					perTargetOutputInfo = &(*it);
					break;
				}

				key = parentTargetName(key);
			}
		}

		{
			QMutexLocker logMutexLocker(&logMutex);

			// handle per system target if any
			if(perTargetOutputInfo) {
				perTargetOutputInfo->outputList.writeToAll(event);
			} else {
				// handle global target
				globalOutputList.writeToAll(event);
			}
		}
	}
	LogLevel loggingLevel(const QString& target) const
	{
		QReadLocker locker(&lock);
		LogLevel level = globalLevel; // by default

		QString key = target;
		LevelPool::const_iterator it;
		while(!key.isEmpty()) {
			it = perTargetLevelPool.find(key);
			if(it!=perTargetLevelPool.end()) {
				level = it->level;
				break;
			}
			key = parentTargetName(key);
		}

		return level;
	}

	mutable QReadWriteLock lock;
	LogLevel globalLevel;
	LevelPool perTargetLevelPool;
	WLogOutputPool perTargetOutputListPool;
	WLogOutputList globalOutputList;

	mutable QMutex logMutex;
};

//static (most unlikely name....)
const QString WLogger::ROOT_KEY = "__root__";

WLogger::WLogger() :
   d(new WLoggerImpl)
{
}

WLogger::~WLogger()
{
   delete d;
}

void WLogger::addLogOutput(WLogOutputPtr& output)
{
	assert(output);
	QWriteLocker locker(&d->lock);
	d->globalOutputList.push_back(output);
}

void WLogger::addLogOutput(const QString& target, WLogOutputPtr& output)
{
	assert(output);
	QWriteLocker locker(&d->lock);
	d->perTargetOutputListPool[target].outputList.push_back(output);
}

void WLogger::setLoggingLevel(LogLevel newLevel)
{
	QWriteLocker locker(&d->lock);
	d->globalLevel = newLevel;
}

void WLogger::setLoggingLevel(const QString& target, LogLevel newLevel)
{
	QWriteLocker locker(&d->lock);
	d->perTargetLevelPool[target] = newLevel;
}

LogLevel WLogger::loggingLevel(void) const
{
	QReadLocker locker(&d->lock);
	return d->globalLevel;
}

LogLevel WLogger::loggingLevel(const QString& target) const
{
	return d->loggingLevel(target);
}

//! creates the complete log message and passes it to the logger
void WLogger::Helper::writeToLog(void)
{
	WLogEvent event(target, level, buffer);
	WLogger& logger = WLogger::instance();
	logger.write(event);
}

WLogger::Helper::~Helper()
{
   try
   {
      writeToLog();
   }
   catch(std::exception& e)
   {
      // you shouldn't throw exceptions from a sink
      Q_UNUSED(e);
      assert(!"exception in logger helper destructor");
      throw;
   }
}

//! sends the message to all the targets
void WLogger::write(const WLogEvent& event)
{
	d->write(event);
}


