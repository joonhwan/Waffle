#include "WSerialPort.h"

WSerialPort::WSerialPort(QObject* parent)
	: QextSerialPort(EventDriven, parent)
{
}

// virtual
WSerialPort::~WSerialPort()
{
}


