#include "wlogguioutput.h"
#include <QMetaType>

WLogGuiOutput::WLogGuiOutput()
{
	qRegisterMetaType<WLogEvent>("WLogEvent");
}

WLogGuiOutput::~WLogGuiOutput()
{
}

//virtual
void WLogGuiOutput::write(const WLogEvent& event)
{
	emit output(event);
}

void WLogGuiOutput::connect(const QObject* receiver, const char* method)
{
	// log output should be serialized by its order no matter which
	// thread writes it(if Qt::AutoConnection were used, any log
	// output in gui thread would be output first than non-gui
	// thread's output
	QObject::connect(this, SIGNAL(output(const WLogEvent&)),
					 receiver, method, Qt::QueuedConnection);
}

