#ifndef MYSM_H
#define MYSM_H

#include <QList>

#include "common/global/WHsm.h"
#include "MyEvents.h"

class MyState;
class WActiveImpl;
class WEventTimer;

class MyStatemachine : public WHsm
{
	Q_OBJECT
public:
	MyStatemachine(WEventDispatcher *parent = 0);
	virtual ~MyStatemachine();
protected:
	int m_busyTimerInterval;
	WEventTimer* m_busyTimer;

	virtual State onInitialize(const WEvent* e);

	State myTop(const WEvent* e);
	State  notReady(const WEvent* e);
	State    uninitialized(const WEvent* e);
	State    error(const WEvent* e);
	State  ready(const WEvent* e);
	State    idle(const WEvent* e);
	State    busy(const WEvent* e);

	QList<WEventPtr> m_internalQueue;

	DECLARE_SM_FUNC(MyStatemachine);
};

#endif // MYSM_H
