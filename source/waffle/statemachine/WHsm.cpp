#include "WHsm.h"
#include "WEvent.h"
#include "WActive.h"

using namespace Wf;

namespace {

WEvent const QEP_reservedEvt_[] = {
	WEvent(Wf::Empty),
    WEvent(Wf::Entry),
    WEvent(Wf::Exit),
    WEvent(Wf::Init)
};

}

/// helper macro to trigger internal event in an HSM
#define QEP_TRIG_(state_, sig_)					\
    ((this->*state_)(&QEP_reservedEvt_[sig_]))

/// helper macro to trigger entry action in an HSM
#define QEP_EXIT_(state_)									\
    if (QEP_TRIG_(state_, Wf::Exit) == HANDLED) {		\
	}

/// helper macro to trigger exit action in an HSM
#define QEP_ENTER_(state_)									\
    if (QEP_TRIG_(state_, Wf::Entry) == HANDLED) {	\
	}

WHsm::WHsm(const QString& name, WActiveImpl* parent)
	: QObject(parent)
	, m_active(parent)
	, m_state(0)
{
	setObjectName(name);
}

void WHsm::initialize(const WEvent *e)
{
    WStateHandler t;

	m_state = &WHsm::initialState;

	// the top-most initial transition must be taken
	State s = (this->*m_state)(e);
	Q_ASSERT(s == TRAN);

    t = (WStateHandler)&WHsm::top;              // HSM starts in the top state
    do {                                           // drill into the target...
        WStateHandler path[Wf::MaxNestPath];
        qint8 ip = (qint8)0;                  // transition entry path index

        path[0] = m_state;
        (void)QEP_TRIG_(m_state, Wf::Empty);
        while (m_state != t) {
            path[++ip] = m_state;
            (void)QEP_TRIG_(m_state, Wf::Empty);
        }
        m_state = path[0];
                                               // entry path must not overflow
        Q_ASSERT(ip < (qint8)Wf::MaxNestPath);

        do {           // retrace the entry path in reverse (desired) order...
            QEP_ENTER_(path[ip]);                            // enter path[ip]
        } while ((--ip) >= (qint8)0);

        t = path[0];                   // current state becomes the new source
    } while (QEP_TRIG_(t, Wf::Init) == TRAN);
    m_state = t;
}

void WHsm::dispatch(const WEvent* e)
{
	// process external 'e'vent first
	process(e);

	// process internal events till internal queue is empty...
	processInternal();
}

bool WHsm::isIn(WStateHandler state)
{
    WStateHandler s = m_state;
    State r;
    quint8 inState = (quint8)0;    // assume that this HSM is not in 'state'
    do {
        if (m_state == state) {                        // do the states match?
            inState = (quint8)1;                  // match found, return TRUE
            r = IGNORED;                        // break out of the loop
        }
        else {
            r = QEP_TRIG_(m_state, Wf::Empty);
        }
    } while (r != IGNORED);                // WHsm_top state not reached
    m_state = s;                                 // restore the original state
    return (inState!=0); // return the status
}

void WHsm::postInternal(const WEventPtr& e)
{
	m_internalEventQueue.push_back(e);
}

void WHsm::postInternal(qint32 eventId)
{
	WEventPtr e(new WEvent(eventId));
	postInternal(e);
}

State WHsm::initialState(const WEvent* e)
{
	return onInitialize(e);
}

State WHsm::top(const WEvent *)
{
    return ignored();                    // the top state ignores all events
}

void WHsm::process(const WEvent *e)
{
	// statemachine MUST be initialized!
	Q_ASSERT(m_state && m_state!=&WHsm::initialState);
	if(!(m_state && m_state!=&WHsm::initialState)) {
		return;
	}

    WStateHandler path[Wf::MaxNestPath];
    WStateHandler s;
    WStateHandler t;
    State r;

    t = m_state;                                     // save the current state

    do {                                // process the event hierarchically...
        s = m_state;
        r = (this->*s)(e);                           // invoke state handler s
    } while (r == SUPER);

    if (r == TRAN) {                                // transition taken?
#ifdef QF_SPY
        WStateHandler src = s;       // save the transition source for tracing
#endif
        qint8 ip = (qint8)(-1);               // transition entry path index
        qint8 iq;                       // helper transition entry path index

        path[0] = m_state;                // save the target of the transition
        path[1] = t;

        while (t != s) {       // exit current state to transition source s...
            if (QEP_TRIG_(t, Wf::Exit) == HANDLED) {   //exit handled?
                (void)QEP_TRIG_(t, Wf::Empty);    // find superstate of t
            }
            t = m_state;                       // m_state holds the superstate
        }

        t = path[0];                               // target of the transition

        if (s == t) {         // (a) check source==target (transition to self)
            QEP_EXIT_(s)                                    // exit the source
            ip = (qint8)0;                                // enter the target
        }
        else {
            (void)QEP_TRIG_(t, Wf::Empty);        // superstate of target
            t = m_state;
            if (s == t) {                   // (b) check source==target->super
                ip = (qint8)0;                            // enter the target
            }
            else {
                (void)QEP_TRIG_(s, Wf::Empty);       // superstate of src
                                     // (c) check source->super==target->super
                if (m_state == t) {
                    QEP_EXIT_(s)                            // exit the source
                    ip = (qint8)0;                        // enter the target
                }
                else {
                                            // (d) check source->super==target
                    if (m_state == path[0]) {
                        QEP_EXIT_(s)                        // exit the source
                    }
                    else { // (e) check rest of source==target->super->super..
                           // and store the entry path along the way
                           //
                        iq = (qint8)0;         // indicate that LCA not found
                        ip = (qint8)1;     // enter target and its superstate
                        path[1] = t;          // save the superstate of target
                        t = m_state;                     // save source->super
                                                  // find target->super->super
                        r = QEP_TRIG_(path[1], Wf::Empty);
                        while (r == SUPER) {
                            path[++ip] = m_state;      // store the entry path
                            if (m_state == s) {           // is it the source?
                                iq = (qint8)1;     // indicate that LCA found
                                               // entry path must not overflow
                                Q_ASSERT(ip < (qint8)Wf::MaxNestPath);
                                --ip;               // do not enter the source
                                r = HANDLED;       // terminate the loop
                            }
                            else {      // it is not the source, keep going up
                                r = QEP_TRIG_(m_state, Wf::Empty);
                            }
                        }
                        if (iq == (qint8)0) {       // the LCA not found yet?

                                               // entry path must not overflow
                            Q_ASSERT(ip < (qint8)Wf::MaxNestPath);

                            QEP_EXIT_(s)                   // exit the source

                            // (f) check the rest of source->super
                            //                  == target->super->super...
                            //
                            iq = ip;
                            r = IGNORED;       // indicate LCA NOT found
                            do {
                                if (t == path[iq]) {       // is this the LCA?
                                    r = HANDLED;   // indicate LCA found
                                    ip = (qint8)(iq - 1); // do not enter LCA
                                    iq = (qint8)(-1);   // terminate the loop
                                }
                                else {
                                    --iq;    // try lower superstate of target
                                }
                            } while (iq >= (qint8)0);

                            if (r != HANDLED) {    // LCA not found yet?
                                // (g) check each source->super->...
                                // for each target->super...
                                //
                                r = IGNORED;             // keep looping
                                do {
                                                          // exit t unhandled?
										if (QEP_TRIG_(t, Wf::Exit)
                                        == HANDLED)
                                    {
                                        (void)QEP_TRIG_(t, Wf::Empty);
                                    }
                                    t = m_state;         //  set to super of t
                                    iq = ip;
                                    do {
                                        if (t == path[iq]) {   // is this LCA?
                                                           // do not enter LCA
                                            ip = (qint8)(iq - 1);
                                            iq = (qint8)(-1);   //break inner
                                            r = HANDLED;   //break outer
                                        }
                                        else {
                                            --iq;
                                        }
                                    } while (iq >= (qint8)0);
                                } while (r != HANDLED);
                            }
                        }
                    }
                }
            }
        }
                       // retrace the entry path in reverse (desired) order...
        for (; ip >= (qint8)0; --ip) {
            QEP_ENTER_(path[ip])                             // enter path[ip]
        }
        t = path[0];                         // stick the target into register
        m_state = t;                               // update the current state

                                         // drill into the target hierarchy...
        while (QEP_TRIG_(t, Wf::Init) == TRAN) {

            ip = (qint8)0;
            path[0] = m_state;
            (void)QEP_TRIG_(m_state, Wf::Empty);       // find superstate
            while (m_state != t) {
                path[++ip] = m_state;
                (void)QEP_TRIG_(m_state, Wf::Empty);   // find superstate
            }
            m_state = path[0];
                                               // entry path must not overflow
            Q_ASSERT(ip < (qint8)Wf::MaxNestPath);

            do {       // retrace the entry path in reverse (correct) order...
                QEP_ENTER_(path[ip])                         // enter path[ip]
            } while ((--ip) >= (qint8)0);

            t = path[0];
        }
    }
    m_state = t;                 // set new state or restore the current state
}

//virtual
void WHsm::processInternal(void)
{
	// do nothing by default
	// any internal event queue handling things should be done here
	while(!m_internalEventQueue.empty()) {
		WEventPtr& e = m_internalEventQueue.first();
		if(e) {
			process(e.data());
		}
		m_internalEventQueue.pop_front();
	}
}

#include "moc_whsm.cxx"

