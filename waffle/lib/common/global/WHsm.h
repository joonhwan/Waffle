#pragma once

// HSM = Hierachical State Machine

#include "common/global/WNamespace.h"
#include "common/global/WEventHandler.h"
#include <QObject>

class WEventDispatcher;
class WHsm;
using Wf::State;
typedef State (WHsm::*WStateHandler)(const WEvent*);

class WHsm : public WEventHandler
{
	Q_OBJECT
	WHsm() : WEventHandler(0) {} // cannot use this ctor.
protected:
public:
	WHsm(const QString& name, WEventDispatcher* dispatcher=0);
	virtual ~WHsm() {};
	// WEventHandler interface
    virtual void initialize(const WEvent *e = (WEvent*)0);
    virtual void dispatch(const WEvent *e);
	// virtual QObject* qobject() { return (QObject*)this; }
    bool isIn(WStateHandler state);

protected:
	virtual State onInitialize(const WEvent* e) = 0;
	void postInternal(const WEventPtr& e);
	void postInternal(qint32 eventId);

	State ignored(void);
	State handled(void);
	State superIsTop(void);

	// default(initial & top) states
	State initialState(const WEvent* e);
	State top(const WEvent *e);

	WStateHandler m_state; ///< current active state
private:
	void process(const WEvent *e);
	void processInternal(void);

	// for handling events for ortogonal region :
	// hint from 'http://drdobbs.com/cpp/196601103'
	QList<WEventPtr> m_internalEventQueue;
};

inline
State WHsm::ignored(void)
{
	return Wf::IGNORED;
}

inline
State WHsm::handled(void)
{
	return Wf::HANDLED;
}

inline
State WHsm::superIsTop(void)
{
	this->m_state = &WHsm::top;
	return Wf::SUPER;
}

#define WLOG_TRANS(e,l)								\
	switch(e->sig)									\
	{												\
	case Wf::Entry:									\
		WLOG_INFO(l) <<  "enter: " << __FUNCTION__;	\
		break;										\
	case Wf::Exit:									\
		WLOG_INFO(l) <<  "enter: " << __FUNCTION__;	\
	}

#define DECLARE_SM_FUNC(className)										\
	typedef State (className::*className##StateHandler)(const WEvent*); \
																		\
	State trans(className##StateHandler state)							\
	{																	\
		this->m_state = (WStateHandler)state;							\
		return Wf::TRAN;												\
	}																	\
	State super(className##StateHandler state)							\
	{																	\
		this->m_state = (WStateHandler)state;							\
		return Wf::SUPER;												\
	}
