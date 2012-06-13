#include "myevents.h"
#include <QMetaEnum>

IMPLEMENT_EVENT_TYPE(MySimpleEvent)

MySimpleEvent::MySimpleEvent(MyEventId::Value id)
	: WEvent(id)
{
}

//virtual
MySimpleEvent::~MySimpleEvent()
{
}

//virtual
QString MySimpleEvent::toString(void) const
{
	QMetaEnum eventIdEnum = MyEventId::staticMetaObject.enumerator(0);
	const char* key = eventIdEnum.valueToKey(this->eventId);
	QString value;
	if(!key) {
		value = QString("id=%1").arg(this->eventId);
	} else {
		value = key;
	}
	return value;
}

IMPLEMENT_EVENT_TYPE(MyValueEvent)

MyValueEvent::MyValueEvent(MyEventId::Value eventId, const QVariant& value)
	: MySimpleEvent(eventId)
	, m_value(value)
{
}

// virtual
MyValueEvent::~MyValueEvent()
{
}
