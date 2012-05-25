#pragma once

#include "common/global/wnamespace.h"
#include "wevent.h"
#include <QList>
#include <QMutex>
#include <QObject>
#include <QTime>


class WHsm;

class WActiveImpl : public QObject
{
    Q_OBJECT
public:
    WActiveImpl();
    virtual ~WActiveImpl();
	virtual bool event(QEvent* e);
	virtual void publish(qint32 eventId, Wf::Priority priority = Wf::Normal);
	void publish(WEventPtr& e, Wf::Priority priority = Wf::Normal);
	void publish(WEvent* e, Wf::Priority priority = Wf::Normal);
	void registerStateMachine(WHsm* sm);
	// no need to unregister.. as for now.
	// void unregisterStateMachine(WHsm* sm);
public slots:
	void initializeStateMachines(void);
	void uninitializeStateMachines(void);
signals:
	void eventPublished(WEventPtr e);
protected slots:
	void handleSingleEvent(void);
protected:
	QMutex m_locker;
	QList<WHsm*> m_smList;

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
// example : QActive<
template<typename T>
class WActive : public WActiveImpl
{
public:
	// simpler publish
	virtual void publish(qint32 eventId, Wf::Priority priority = Wf::Normal) {
		WEventPtr e(new T((T::IdType)eventId));
		WActiveImpl::publish(e, priority);
	}
};

