#pragma once

#include "statemachine/wevent.h"
#include "MyEventIds.h"
#include <QVariant>

class MySimpleEvent : public WEvent
{
public:
	typedef MyEventId::Value IdType;
	DECLARE_EVENT_TYPE(MySimpleEvent);
	MySimpleEvent(MyEventId::Value id);
	virtual ~MySimpleEvent();
	virtual QString toString(void) const;
};

class MyValueEvent : public MySimpleEvent
{
public:
	DECLARE_EVENT_TYPE(MyValueEvent)
	MyValueEvent(MyEventId::Value id, const QVariant& value);
	virtual ~MyValueEvent();
	const QVariant& value(void) const
	{
		return m_value;
	}
protected:
	QVariant m_value;
};


// template <class T> inline T qgraphicsitem_cast(QGraphicsItem *item)
// {
//     return int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type)
//         || (item && int(static_cast<T>(0)->Type) == item->type()) ? static_cast<T>(item) : 0;

// }
