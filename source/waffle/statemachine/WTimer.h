#pragma once

#include "common/global/wglobal.h"
#include <QTimer>


class WHsm;

// NOTE: inherit implementaion only from QTimer.(derived
// 'protected'ly)
class WTimerImpl : protected QTimer
{
	Q_OBJECT
public:
	WTimerImpl(WHsm* sm);
	WTimerImpl& singleShot(int interval);
	WTimerImpl& periodic(int interval);
	void arm(int eventId);
	void disarm(void);
protected:
	WHsm* m_sm;
	int m_eventId;
	virtual void timerEvent(QTimerEvent* e);
};

// T is simple type that client provides
// T type should have following items
//   - typedef of event id
//   - constructor that accept event id
template<typename T>
class WTimer : public WTimerImpl
{
public:
	WTimer(WHsm* sm)
		: WTimerImpl(sm)
	{
	}
	//virtual
	void timerEvent(QTimerEvent* e)
	{
		// no need to do 'emit timeout()' in
		// QTimer::timerEvent(e)
		// just skip
		if(e->timerId()==timerId()) {
			if(m_sm) {
				T timerEvent((T::IdType)m_eventId);
				m_sm->dispatch(&timerEvent);
			}
			if(isSingleShot()){
				stop();
			}
		}
	}
};

