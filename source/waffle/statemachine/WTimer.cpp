#include "wtimer.h"
#include "whsm.h"
#include "wevent.h"
#include <QTimerEvent>


WTimerImpl::WTimerImpl(WHsm* sm)
	: QTimer(sm)
	, m_sm(sm)
{
}

WTimerImpl& WTimerImpl::singleShot(int interval)
{
	this->setSingleShot(true);
	this->setInterval(interval);
	return *this;
}

WTimerImpl& WTimerImpl::periodic(int interval)
{
	this->setSingleShot(false);
	this->setInterval(interval);
	return *this;
}

void WTimerImpl::arm(int eventId)
{
	m_eventId = eventId;
	QTimer::start();
}

void WTimerImpl::disarm(void)
{
	QTimer::stop();
}

//virtual
void WTimerImpl::timerEvent(QTimerEvent* e)
{
	// no need to do 'emit timeout()' in
	// QTimer::timerEvent(e)
	// just skip
	if(e->timerId()==timerId()) {
		if(m_sm) {
			WEvent timerEvent(m_eventId);
			m_sm->dispatch(&timerEvent);
		}
		if(isSingleShot()){
			stop();
		}
	}
}

#include "moc_wtimer.cxx"

