#pragma once

#include "common/global/WGlobal.h"
#include "common/global/WEventHandler.h"
#include <QTimer>

class WEventHandler;

// NOTE: inherit implementaion only from QTimer.(derived
// 'protected'ly)
class WEventTimer : protected QTimer
{
	Q_OBJECT
public:
	WEventTimer(WEventHandler* handler);
	WEventTimer& singleShot(int interval);
	WEventTimer& periodic(int interval);
	void arm(int eventId);
	void disarm(void);
protected:
	WEventHandler* m_handler;
	int m_eventId;
	virtual void timerEvent(QTimerEvent* e);
};

// T is simple type that client provides
// T type should have following items(see below [A])
//   - typedef of event id(enum IdType!)
//   - constructor that accept event id
template<typename T>
class WEventTimerT : public WEventTimer
{
public:
	WEventTimerT(WEventHandler* handler)
		: WEventTimer(handler)
	{
	}
	virtual void timerEvent(QTimerEvent* e)
	{
		// no need to do 'emit timeout()' in
		// QTimer::timerEvent(e)
		// just skip
		if(e->timerId()==timerId()) {
			if(m_handler) {
				// [A]. see above
				T timerEvent((T::IdType)m_eventId);
				m_handler->dispatch(&timerEvent);
			}
			if(isSingleShot()){
				stop();
			}
		}
	}
};

