#include "WEventHandler.h"
#include "WEventDispatcher.h"

WEventHandler::WEventHandler(WEventDispatcher* dispatcher)
	: QObject(dispatcher)
	, m_dispatcher(dispatcher)
{
	if (dispatcher) {
		dispatcher->registerHandler(this);
	}
}

//virtual
WEventHandler::~WEventHandler()
{
}

