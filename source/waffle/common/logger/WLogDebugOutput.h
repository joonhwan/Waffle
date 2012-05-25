#pragma once

#include "common/global/wglobal.h"
#include "common/logger/wlogoutput.h"


class WLogDebugOutput : public WLogOutput
{
public:
	virtual void write(const WLogEvent& event);
};

