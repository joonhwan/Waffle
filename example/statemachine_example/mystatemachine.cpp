#include "mystatemachine.h"
#include "common/global/WEventTimer.h"
#include "common/global/WEventDispatcher.h"
#include "common/logger/WLogger.h"

#include <QDebug>

const char* smLogger = "MySM";

MyStatemachine::MyStatemachine(WEventDispatcher *parent)
	: WHsm("MyStatemachine", parent)
	, m_busyTimerInterval(1000)
{
	// top = new MyState(0, "top");
	// notReady = new MyState(top, "notReady");
	// initial = new MyState(notReady, "initial");
	// error = new MyState(notReady, "error");
	// ready = new MyState(top, "ready");
	// idle = new MyState(ready, "idle");
	// busy = new MyState(ready, "busy");

	// addState(top);

	m_busyTimer = new WEventTimerT<MySimpleEvent>(this);
}

// virtual
MyStatemachine::~MyStatemachine()
{
}

//virtual
Wf::State MyStatemachine::onInitialize(const WEvent* e)
{
	return trans(&MyStatemachine::notReady);
}

Wf::State MyStatemachine::myTop(const WEvent* e)
{
	switch(e->sig)
	{
	case MyEventId::ENTRY_STATE:
		return handled();
	case MyEventId::INIT_STATE:
		return handled();
	case MyEventId::EXIT_STATE:
		return handled();
	case MyEventId::ERROR:
		return trans(&MyStatemachine::error);
	case MyEventId::TERM_STATEMACHINE:
		return trans(&MyStatemachine::uninitialized);
	}

	return superIsTop();
}

Wf::State  MyStatemachine::notReady(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	switch(e->sig)
	{
	case MyEventId::ENTRY_STATE:
		return handled();
	case MyEventId::INIT_STATE:
		return trans(&MyStatemachine::uninitialized);
	case MyEventId::EXIT_STATE:
		return handled();

	case MyEventId::INIT:
		return trans(&MyStatemachine::ready);
	}

	return super(&MyStatemachine::myTop);
}

Wf::State    MyStatemachine::uninitialized(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	switch(e->sig)
	{
	case MyEventId::ENTRY_STATE:
		return handled();
	case MyEventId::INIT_STATE:
		return handled();
	case MyEventId::EXIT_STATE:
		return handled();

	case MyEventId::TERM_STATEMACHINE:
		WLOG_INFO(smLogger) << "ignored redundant terminate.";
		return handled();
	}

	return super(&MyStatemachine::notReady);
}

Wf::State    MyStatemachine::error(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	switch(e->sig)
	{
	case MyEventId::ENTRY_STATE:
		WLOG_ERROR(smLogger) << "system down!";
		return handled();
	case MyEventId::INIT_STATE:
		return handled();
	case MyEventId::EXIT_STATE:
		return handled();
	}

	return super(&MyStatemachine::notReady);
}

Wf::State  MyStatemachine::ready(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	switch(e->sig) {
	case MyEventId::ENTRY_STATE:
		return handled();
	case MyEventId::INIT_STATE:
		return trans(&MyStatemachine::idle);
	case MyEventId::EXIT_STATE:
		return handled();

	case MyEventId::BUSY_TIMER_INTERVAL_CHANGE: {
		MyValueEvent* ve = (MyValueEvent*)e;
		m_busyTimerInterval = ve->value().toInt();
		return handled();
	}
	}

	return super(&MyStatemachine::myTop);
}

Wf::State    MyStatemachine::idle(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	switch(e->sig)
	{
	case MyEventId::ENTRY_STATE:
		return handled();
	case MyEventId::INIT_STATE:
		return handled();
	case MyEventId::EXIT_STATE:
		return handled();

	case MyEventId::BEGIN:
		return trans(&MyStatemachine::busy);
	}

	return super(&MyStatemachine::ready);
}

Wf::State    MyStatemachine::busy(const WEvent* e)
{
	WLOG_TRANS(e, smLogger);

	static int busyCounter = 0;
	static QTime stopwatch;

	switch(e->sig) {
	case MyEventId::ENTRY_STATE: {
		busyCounter = 0;
		postInternal(MyEventId::BUSY_TIMER_START);
		return handled();
	}
	case MyEventId::INIT_STATE: {
		return handled();
	}
	case MyEventId::EXIT_STATE: {
		m_busyTimer->disarm();
		return handled();
	}
	case MyEventId::END: {
		return trans(&MyStatemachine::idle);
	}
	case MyEventId::BUSY_TIMER_INTERVAL_CHANGE: {
		MyValueEvent* ve = (MyValueEvent*)e;
		m_busyTimerInterval = ve->value().toInt();
		postInternal(MyEventId::BUSY_TIMER_START);
		return handled();
	}
	case MyEventId::BUSY_TIMER_START: {
		m_busyTimer->periodic(m_busyTimerInterval).arm(MyEventId::BUSY_TIMEOUT);
		stopwatch.start();
		return handled();
	}
	case MyEventId::BUSY_TIMEOUT: {
		WLOG_INFO(smLogger) << "timeout!" << busyCounter++ << " : "
							<< stopwatch.restart() << " msec";
		// for(int i=0;i<1000;++i) {
		// 	m_active->publish(MyEventId::PING, Wf::Lazy);
		// }
		return handled();
	}
	case MyEventId::PING:
		WLOG_TRACE(smLogger) << "ping!";
		break;
	}

	return super(&MyStatemachine::ready);
}
