#pragma once

#include "common/global/WEvent.h"
#include <QObject>

class WEventDispatcher;

class WEventHandler : public QObject
{
	Q_OBJECT
public:
    WEventHandler(WEventDispatcher* dispatcher);
    virtual ~WEventHandler();
	virtual void initialize(const WEvent* e = 0) = 0;
	virtual void dispatch(const WEvent* e) = 0;
protected:
	WEventDispatcher* m_dispatcher;
};
