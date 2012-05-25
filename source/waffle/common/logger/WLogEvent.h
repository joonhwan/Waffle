#pragma once

#include "common/global/wnamespace.h"
#include <QMetaType>
#include <QString>
#include <QDateTime>

class WLogEvent
{
public:
	WLogEvent();
    WLogEvent(const QString& _target,
			  Wf::LogLevel  _level,
			  const QString& _log);
    virtual ~WLogEvent();
	QString logMessage(void) const ;
	QDateTime now;
	QString target;
	Wf::LogLevel level;
	Qt::HANDLE tid;
	QString levelName;
	QString threadName;
	QString log;
};

Q_DECLARE_METATYPE(WLogEvent)

