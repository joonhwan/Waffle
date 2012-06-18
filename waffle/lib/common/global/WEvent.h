/***************************************************************************
 **
 ** Copyright (C) 2010 WisePlanet Co. Ltd. and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: WisePlanet (sw@wiseplanet.co.kr)
 **
 ****************************************************************************/
#pragma once

#include "common/global/wglobal.h"
#include <QEvent>
#include <QString>
#include <QSharedPointer>


#define DECLARE_EVENT_TYPE(className)			\
	static const int myType;					\
	virtual int type(void) const				\
	{											\
		return WEvent::myType;					\
	}

#define IMPLEMENT_EVENT_TYPE(className)							\
	const int className::myType = QEvent::registerEventType();	\

class WEvent
{
public:
	DECLARE_EVENT_TYPE(WEvent)
	WEvent(qint32 eventId);
	virtual ~WEvent();
	virtual QString toString(void) const;
	bool ValidSystemId(void) const { return systemId>=0; }
	union
	{
		qint32 eventId;
		qint32 sig; // tranditional name of event id. ;)
	};
	int systemId; // used as 'system id' hint if neccessary
};

typedef QSharedPointer<WEvent> WEventPtr;

