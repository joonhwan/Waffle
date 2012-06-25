#pragma once

#include <QObject>
#include <QEventLoop>

// this class was hooked from libQxt's QxtSignalWaiter.
//
// The WQtSignalWaiter class blocks and processes events until a signal is emitted
// In many cases, writing code that assumes certain actions are synchronous is considerably simpler than breaking your function into
// multiple blocks and using signals and slots to connect them all. Using this class, QSignalWaiter::wait will block until a certain
// signal is emitted and then return. The return value is true if the signal was caught, or false if a user-specified timeout elapses
// before catching the signal.
//
// void MyObject::myFunction() {
//     WQtSignalWaiter waiter(myOtherObject, SIGNAL(longProcessFinished()));
//     myOtherObject->longProcess();
//     if(waiter.wait(5000)) {
//         doSomething(myOtherObject->information());
//     } else {
//         QMessageBox::information(0, "MyObject", "Timed out while waiting on longProcessFinished()", QMessageBox::Ok);
//     }
// }
//
// WQtSignalWaiter is not reentrant. In particular, only one WQtSignalWaiter object per thread can be safely waiting at a
// time. If a second WQtSignalWaiter is used while the first is waiting, the first will not return until the second has
// timed out or successfully caught its signal.
//
// 한 쓰레드에 1개의 WQtSignalWaiter만 가능.
// Reentrant하지 않음.
class WQtSignalWaiter : public QObject
{
    Q_OBJECT
public:
    WQtSignalWaiter(const QObject* sender, const char* signal);
    static bool wait(const QObject* sender, const char* signal, int msec = -1, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    bool wait(int msec = -1, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    bool hasCapturedSignal() const;
public Q_SLOTS:
    void signalCaught();
    void cancelWait();
private:
    void timerEvent(QTimerEvent* event);
	void stopTimer();

	bool ready, timeout, emitted, waiting;
    int timerID;
};
