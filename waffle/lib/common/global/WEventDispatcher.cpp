#include "WEventDispatcher.h"
#include "WEventHandler.h"
#include "common/logger/wlogger.h"

#include <QDebug>
#include <QMetaType>
#include <QMutexLocker>

namespace {

const char* logger = "Dispatcher";

}


WEventDispatcher::WEventDispatcher()
	: QObject(0)
{
	qRegisterMetaType<WEventPtr>("WEventPtr");
	connect(this, SIGNAL(eventPublished(WEventPtr)),
			SLOT(handleSingleEvent()),
			Qt::QueuedConnection // always 'queued' executation!
		);
}

//virtual
WEventDispatcher::~WEventDispatcher()
{
	// just in case
	uninitializeStateMachines();
}

//virtual
void WEventDispatcher::publish(qint32 eventId, Wf::Priority priority)
{
	WEventPtr e(new WEvent(eventId));
	publish(e, priority);
}

void WEventDispatcher::publish(WEventPtr& e, Wf::Priority priority)
{
	QMutexLocker lock(&m_locker);
	bool inserted = false;

	EventQueueItem eqi; // the new queue item.
	eqi.stamp.start(); // = QTime::currentTime();
	eqi.event = e;
	eqi.priority = priority;

	// if last item's priority is already lower than 'e' event,
	// then no need to find insert position(can just push_back)
	if(m_eventQueue.size() &&
	   (int)priority > (int)m_eventQueue.last().priority) {
		int index = 0;
		EventQueue::iterator it = m_eventQueue.begin();
		for(;it!=m_eventQueue.end();++it, ++index) {
			if((int)priority > (int)(*it).priority) {
				m_eventQueue.insert(it, eqi);
				WLOG_TRACE(logger)
					<< "event[" << eqi.event->toString() << "]"
					<< " queued at " << index << " mid  position.(count="
					<< m_eventQueue.size() << ")";
				inserted = true;
				break;
			}
		}
		Q_ASSERT(inserted); // how could it be?
	}

	// fallback case OR last item's priority is at least lower
	// than 'e' event.
	if(!inserted) {
		m_eventQueue.push_back(eqi);
		WLOG_TRACE(logger)
			<< "event[" << eqi.event->toString() << "]"
			<< " queued at " << m_eventQueue.size()-1 << " last position. (count="
			<< m_eventQueue.size() << ")";
	}

	emit eventPublished(e);
}

void WEventDispatcher::publish(WEvent* _e, Wf::Priority priority)
{
	WEventPtr e(_e);
	publish(e, priority);
}

void WEventDispatcher::registerHandler(WEventHandler* handler)
{
	QMutexLocker lock(&m_locker);
	m_handlerList.push_back(handler);
}

void WEventDispatcher::initializeStateMachines()
{
	QMutexLocker lock(&m_locker);
	foreach(WEventHandler* handler, m_handlerList) {
		handler->initialize(0);
	}
}

void WEventDispatcher::uninitializeStateMachines()
{
	QMutexLocker lock(&m_locker);
	WEvent e(Wf::Term);
	foreach(WEventHandler* handler, m_handlerList) {
		handler->dispatch(&e);
	}
}

void WEventDispatcher::handleSingleEvent()
{
	QMutexLocker lock(&m_locker);

	if(m_eventQueue.size()) {
		EventQueueItem eqi = m_eventQueue.first();
		m_eventQueue.pop_front();
		lock.unlock();

		foreach(WEventHandler* handler, m_handlerList) {
			handler->dispatch(eqi.event.data());
		}

		WLOG_TRACE(logger)
			<< "event[" << eqi.event->toString() << "]"
			<< " processed after " << eqi.stamp.elapsed() << " msec.";
	}
}

// #include "moc_wactive.cxx"

