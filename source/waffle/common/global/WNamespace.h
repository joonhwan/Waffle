#ifndef _WNAMESPACE_H_
#define _WNAMESPACE_H_

#include "common/global/wglobal.h"
#include <QObject>

namespace Wf {

enum Priority {
	VeryUrgent = 2,
	Urgent = 1,
	Normal = 0,
	Lazy = -1,
	VeryLazy = -2
};
enum ReservedEventIds {
	Empty = 0,
	Entry,
	Exit,
	Init,
	Term,
	User
};
enum Config {
	MaxNestPath = 10
};
enum State {
	/// \brief Value returned by a state-handler function when it handles
	/// the event.
	HANDLED = 0,
	/// \brief Value returned by a non-hierarchical state-handler function when
	/// it ignores (does not handle) the event.
	IGNORED,
	/// \brief Value returned by a state-handler function when it takes a
	/// regular state transition.
	TRAN,
	/// \brief Value returned by a state-handler function when it cannot
	/// handle the event.
	SUPER,
};

enum LogLevel
{
	TraceLevel = 0,
	DebugLevel,
	InfoLevel,
	WarnLevel,
	ErrorLevel,
	FatalLevel,

	LogLevelCount
};

}

// placeholder for enums used in Waffle framework
class WfConstant : public QObject
{
	Q_OBJECT
	Q_ENUMS(Wf::Priority)
	Q_ENUMS(Wf::ReservedEventIds)
	Q_ENUMS(Wf::LogLevel)
public:
};

#endif /* _WNAMESPACE_H_ */
