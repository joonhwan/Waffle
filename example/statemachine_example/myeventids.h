#pragma once

#include "common/global/wnamespace.h"
#include <QObject>

class MyEventId : public QObject
{
    Q_OBJECT
	Q_ENUMS(Value)
public:
	enum Value {
		ENTRY_STATE = Wf::Entry,
		INIT_STATE = Wf::Init,
		EXIT_STATE = Wf::Exit,
		TERM_STATEMACHINE = Wf::Term,

		INIT = Wf::User,
		READYED,
		BEGIN,
		END,
		ERROR,
		PING,
		BUSY_TIMEOUT,
		BUSY_TIMER_INTERVAL_CHANGE,
		BUSY_TIMER_START,
	};
};

