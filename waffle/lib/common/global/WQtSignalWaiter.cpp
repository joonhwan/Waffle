/*!
\class WQtSignalWaiter

\inmodule QxtCore

\brief The WQtSignalWaiter class blocks and processes events until a signal is emitted

In many cases, writing code that assumes certain actions are synchronous is considerably simpler than breaking your function into
multiple blocks and using signals and slots to connect them all. Using this class, QSignalWaiter::wait will block until a certain
signal is emitted and then return. The return value is true if the signal was caught, or false if a user-specified timeout elapses
before catching the signal.

\code
void MyObject::myFunction() {
    WQtSignalWaiter waiter(myOtherObject, SIGNAL(longProcessFinished()));
    myOtherObject->longProcess();
    if(waiter.wait(5000)) {
        doSomething(myOtherObject->information());
    } else {
        QMessageBox::information(0, "MyObject", "Timed out while waiting on longProcessFinished()", QMessageBox::Ok);
    }
}
\endcode

\bold {Note:}
WQtSignalWaiter is not reentrant. In particular, only one WQtSignalWaiter object per thread can be safely waiting at a
time. If a second WQtSignalWaiter is used while the first is waiting, the first will not return until the second has
timed out or successfully caught its signal.
*/

#include "WQtSignalWaiter.h"
#include <QCoreApplication>
#include <QTimerEvent>

/*!
 * Constructs a WQtSignalWaiter that will wait for \a signal from \a sender ie. sender::signal()
 * to be emitted. WQtSignalWaiter objects are intended to be created on the stack, therefore no
 * parent parameter is accepted.
 */
WQtSignalWaiter::WQtSignalWaiter(const QObject* sender, const char* signal)
	: QObject(0)
	, ready(false)
	, emitted(false)
	, timeout(false)
	, waiting(false)
{
    Q_ASSERT(sender && signal);
    connect(sender, signal, this, SLOT(signalCaught()));
}

/*!
 * This is an overloaded function provided for convenience. This version can be invoked without first instantiating
 * a WQtSignalWaiter object. Waits for \a signal from \a sender to be emitted within \a msec while processing events
 * according to \a flags. Returns \c true if the signal was caught, or \c false if the timeout elapsed.
 */
bool WQtSignalWaiter::wait(const QObject* sender, const char* signal, int msec, QEventLoop::ProcessEventsFlags flags)
{
    WQtSignalWaiter w(sender, signal);
    return w.wait(msec, flags);
}

/*!
 * Blocks the current function until sender::signal() is emitted. If msec is not -1, wait() will return before the
 * signal is emitted if the specified number of milliseconds have elapsed.
 * Returns \c true if the signal was caught, or \c false if the timeout elapsed.
 * Note that wait() may continue to block after the signal is emitted or the timeout elapses; the function only
 * guarantees that it will not return BEFORE one of these conditions has occurred. This function is not reentrant.
 */
bool WQtSignalWaiter::wait(int msec, QEventLoop::ProcessEventsFlags flags)
{
    // Clear the emission status
    this->ready = false;
    this->emitted = false;

    // Check input parameters
    if (msec < -1 || msec == 0)
        return false;

    // activate the timeout
    if (msec != -1)
        this->timerID = startTimer(msec);
    else
        this->timerID = 0;

    // Make sure to wait for events
    flags |= QEventLoop::WaitForMoreEvents;

    // Begin waiting
    this->waiting = true;
    while (!this->ready && !this->timeout)
        QCoreApplication::processEvents(flags);

    // Clean up and return status
    this->stopTimer();
    this->emitted = this->ready;
    this->waiting = false;
    return this->ready;
}

/*!
 * Returns \c true if the desired signal was emitted during the last wait() call.
 */
bool WQtSignalWaiter::hasCapturedSignal() const
{
    return this->emitted;
}

/*!
 * Signals a waiting object to stop blocking because the desired signal was emitted.
 * WQtSignalWaiter::hasCapturedSignal() will return true after this slot is invoked.
 * Use this slot to allow WQtSignalWaiter to wait for the first of multiple signals.
 */
void WQtSignalWaiter::signalCaught()
{
    if (!this->waiting) return;
    this->ready = true;
    this->stopTimer();
}

/*!
 * \reimp
 */
void WQtSignalWaiter::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    cancelWait();
}

void WQtSignalWaiter::stopTimer()
{
	if (timerID)
		this->killTimer(timerID);
	timerID = 0;
	waiting = false;
}


/*!
 * Signals a waiting object to stop blocking because the timeout has elapsed.
 * WQtSignalWaiter::hasCapturedSignal() will return false after this slot is invoked.
 * Use this slot to allow WQtSignalWaiter to be interrupted for reasons other than
 * a timeout.
 */
void WQtSignalWaiter::cancelWait()
{
    if (!this->waiting) return;
    this->timeout = true;
    this->stopTimer();
}
