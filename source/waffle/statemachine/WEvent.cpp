#include "WEvent.h"


IMPLEMENT_EVENT_TYPE(WEvent)

WEvent::WEvent(qint32 _eventId)
	: eventId(_eventId)
	, systemId(-1)
{
}

//virtual
WEvent::~WEvent()
{
}

//virtual
QString WEvent::toString(void) const
{
	QString d = QString("event[id=%1, sys=%2]").arg(eventId).arg(systemId);
	return d;
}

