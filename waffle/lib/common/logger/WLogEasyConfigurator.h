#pragma once

#include "common/global/WNamespace.h"

class WLogEasyConfigurator
{
public:
	WLogEasyConfigurator(Wf::LogLevel defaultLogLevel = Wf::InfoLevel);
	virtual ~WLogEasyConfigurator();
	void initializeLog(void);
protected:
	QString m_defaultLogFilePath;
	Wf::LogLevel m_defaultLogLevel;
};
