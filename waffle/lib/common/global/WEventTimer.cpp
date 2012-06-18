#include "WEventTimer.h"
#include "WEventHandler.h"
#include "WHsm.h"
#include "WEvent.h"
#include <QTimerEvent>

WEventTimer::WEventTimer(WEventHandler* handler)
	: QTimer(handler)
	, m_handler(handler)
{
	// if (handler && handler->qobject()) {
	// 	setParent(handler->qobject());
	// }
}

WEventTimer& WEventTimer::singleShot(int interval)
{
	this->setSingleShot(true);
	this->setInterval(interval);
	return *this;
}

WEventTimer& WEventTimer::periodic(int interval)
{
	this->setSingleShot(false);
	this->setInterval(interval);
	return *this;
}

void WEventTimer::arm(int eventId)
{
	m_eventId = eventId;
	QTimer::start();
}

void WEventTimer::disarm(void)
{
	QTimer::stop();
}

//virtual
void WEventTimer::timerEvent(QTimerEvent* e)
{
	// no need to do 'emit timeout()' in
	// QTimer::timerEvent(e)
	// just skip
	if(e->timerId()==timerId()) {
		if(m_handler) {
			WEvent timerEvent(m_eventId);
			m_handler->dispatch(&timerEvent);
		}
		if(isSingleShot()){
			stop();
		}
	}
}

// #include "moc_wtimer.cxx"

