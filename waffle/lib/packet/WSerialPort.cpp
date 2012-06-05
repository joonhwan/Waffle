#include "WSerialPort.h"

WSerialPort::WSerialPort(QueryMode mode, QObject* parent)
	: QextSerialPort(mode, parent)
{
}

// virtual
WSerialPort::~WSerialPort()
{
}


