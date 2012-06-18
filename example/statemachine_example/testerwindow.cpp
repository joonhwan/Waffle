#include "testerwindow.h"
#include "mystatemachine.h"
#include "myeventids.h"
#include "myevents.h"
#include "common/global/WEventDispatcher.h"
#include "common/logger/wlogger.h"
#include "common/logger/wlogguioutput.h"
#include "gui/wlogview.h"
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QSpinBox>
#include <QThread>
#include <QVBoxLayout>
#include <QWaitCondition>

const char* uiLogger = "GUI";

TesterWindow::TesterWindow(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
	, m_thread(0)
	, m_logOutput(new WLogGuiOutput)
{
	WLogger::instance().addLogOutput(m_logOutput);
	m_logOutput->connect(this, SLOT(displayLog(const WLogEvent&)));

	setAttribute(Qt::WA_QuitOnClose, true);

	startButton = new QPushButton(tr("Start"));
	stopButton = new QPushButton(tr("Stop"));
	QPushButton* initButton = new QPushButton(tr("Init"));
	QPushButton* beginWork = new QPushButton(tr("Begin"));
	QPushButton* endWork = new QPushButton(tr("End"));
	QPushButton* ping = new QPushButton(tr("Ping"));
	QPushButton* boom = new QPushButton(tr("Boom"));

	QSpinBox* interval = new QSpinBox();
	interval->setValue(1000);
	interval->setMaximum(5000);
	interval->setMinimum(20);

	// QPushButton* lengthyWorkButton = new QPushButton(tr("lengthyWork"));
	QLabel* stateLabelTitle = new QLabel(tr("State:"));
	stateLabel = new QLabel();

	logView = new WLogView();
	m_logOutput->connect(logView, SLOT(addLog(const WLogEvent&)));

	QHBoxLayout* layout1 = new QHBoxLayout;
	layout1->addWidget(startButton);
	layout1->addWidget(stopButton);

	QHBoxLayout* layout1_1 = new QHBoxLayout;
	layout1_1->addWidget(initButton);
	layout1_1->addWidget(beginWork);
	layout1_1->addWidget(endWork);
	layout1_1->addWidget(ping);
	layout1_1->addWidget(boom);

	QHBoxLayout* layout2 = new QHBoxLayout;
	layout2->addWidget(stateLabelTitle);
	layout2->addWidget(stateLabel);

	QHBoxLayout* layout3 = new QHBoxLayout;
	layout3->addWidget(new QLabel(tr("Busy Timer Interval")));
	layout3->addWidget(interval);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(layout1);
	layout->addLayout(layout1_1);
	layout->addLayout(layout2);
	layout->addWidget(logView);
	layout->addLayout(layout3);
	setLayout(layout);

	m_thread = new QThread(this);
	m_thread->setObjectName("SmThread");
	{
		dispatcher = new WEventDispatcherT<MySimpleEvent>();
		{
			sm = new MyStatemachine(dispatcher);
		}
		dispatcher->moveToThread(m_thread);
	}
	m_thread->start();

	connect(startButton, SIGNAL(clicked()), dispatcher, SLOT(initializeStateMachines()));
	// connect(startButton, SIGNAL(clicked()), this, SLOT(onBegin()));
	connect(stopButton, SIGNAL(clicked()), dispatcher, SLOT(uninitializeStateMachines()));

	QSignalMapper* mapper = new QSignalMapper(this);

	connect(initButton, SIGNAL(clicked()), mapper, SLOT(map()));
	mapper->setMapping(initButton, MyEventId::INIT);
	connect(beginWork, SIGNAL(clicked()), mapper, SLOT(map()));
	mapper->setMapping(beginWork, MyEventId::BEGIN);
	connect(endWork, SIGNAL(clicked()), mapper, SLOT(map()));
	mapper->setMapping(endWork, MyEventId::END);
	connect(boom, SIGNAL(clicked()), mapper, SLOT(map()));
	mapper->setMapping(boom, MyEventId::ERROR);
	connect(ping, SIGNAL(clicked()), mapper, SLOT(map()));
	mapper->setMapping(ping, MyEventId::PING);

	connect(mapper, SIGNAL(mapped(int)), SLOT(operationClicked(int)));

	connect(interval, SIGNAL(valueChanged(int)), SLOT(intervalChanged(int)));
}

TesterWindow::~TesterWindow()
{
	if(m_thread) {
		m_thread->exit();
	}
	delete dispatcher;
}

void TesterWindow::onInit(void)
{
}

void TesterWindow::onBegin(void)
{
}

void TesterWindow::operationClicked(int eventId)
{
	if(eventId==MyEventId::PING) {
		dispatcher->publish(eventId, Wf::Lazy);
	} else {
		dispatcher->publish(eventId, Wf::Urgent);
	}
}

void TesterWindow::intervalChanged(int interval)
{
	dispatcher->publish(new MyValueEvent(MyEventId::BUSY_TIMER_INTERVAL_CHANGE, interval));
}
