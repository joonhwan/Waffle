#pragma once

#include "common/global/wglobal.h"
#include "common/logger/wlogoutput.h"

#include <QObject>


class WLogGuiOutput : public QObject
					, public WLogOutput
{
	Q_OBJECT
public:
    WLogGuiOutput();
    virtual ~WLogGuiOutput();
	// WLogOutput interface
	virtual void write(const WLogEvent& event);
	virtual void connect(const QObject* receiver, const char* method);
signals:
	void output(const WLogEvent& event);
protected:
};

