#ifndef LOGDEST_H
#define LOGDEST_H

#include "common/global/wglobal.h"
#include "common/logger/wlogevent.h"
#include <QSharedPointer>

class QString;


class WLogOutput
{
public:
	WLogOutput();
	virtual ~WLogOutput();
	virtual void write(const WLogEvent& event) = 0;
	virtual void connect(const QObject* receiver, const char* slot);
};

typedef QSharedPointer<WLogOutput> WLogOutputPtr;


#endif // QSLOGDEST_H
