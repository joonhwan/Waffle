#pragma once

#include "common/global/WNamespace.h"
#include "common/global/WEvent.h"
#include "common/global/WEventHandler.h"
#include <QList>
#include <QMutex>
#include <QObject>
#include <QObject>
#include <QTime>

class WEventHandler;

class WEventDispatcher : public QObject
{
    Q_OBJECT
public:
    WEventDispatcher();
    virtual ~WEventDispatcher();
	virtual void publish(qint32 eventId, Wf::Priority priority = Wf::Normal);
	void publish(WEventPtr& e, Wf::Priority priority = Wf::Normal);
	void publish(WEvent* e, Wf::Priority priority = Wf::Normal);
	// NOTE: register() does not take ownership! delete it by yourself.
	void registerHandler(WEventHandler* handler);
	// no need to unregister.. as for now.
	// void unregister(WEventHandler* handler);
public slots:
	void initializeStateMachines(void);
	void uninitializeStateMachines(void);
signals:
	void eventPublished(WEventPtr e);
protected slots:
	void handleSingleEvent(void);
protected:
	QMutex m_locker;
	QList<WEventHandler*> m_handlerList;

	struct EventQueueItem
	{
		Wf::Priority priority;
		QTime stamp;
		WEventPtr event;
	};
	typedef QList<EventQueueItem> EventQueue;
	EventQueue m_eventQueue;
};

// T should be top level event type for each client.
// example : WEventDispatcherT<MyBaseEventType>
template<typename T>
class WEventDispatcherT : public WEventDispatcher
{
public:
	// simpler publish
	virtual void publish(qint32 eventId, Wf::Priority priority = Wf::Normal) {
		WEventPtr e(new T((T::IdType)eventId));
		WEventDispatcher::publish(e, priority);
	}
};
