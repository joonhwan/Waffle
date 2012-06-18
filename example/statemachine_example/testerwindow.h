#ifndef QTSMTEST_H
#define QTSMTEST_H

#include "common/logger/wlogoutput.h"
#include <QWidget>
#include <QState>

class QPushButton;
class QLabel;
class QThread;
class MyStatemachine;

class WEventDispatcher;
class WLogEvent;
class WLogView;

class TesterWindow : public QWidget
{
	Q_OBJECT
public:
	TesterWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~TesterWindow();
public Q_SLOTS:
	void onInit(void);
	void onBegin(void);
	void operationClicked(int eventId);
	void intervalChanged(int internval);
protected:
	QPushButton* startButton;
	QPushButton* stopButton;
	QLabel* stateLabel;
	WLogView* logView;
	MyStatemachine* sm;
	WEventDispatcher* dispatcher;
	QThread* m_thread;
	WLogOutputPtr m_logOutput;
};

#endif // QTSMTEST_H
