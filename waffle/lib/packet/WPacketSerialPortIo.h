#pragma once

#include "packet/WSerialPort.h"
#include "packet/WPacketIoT.h"
#include <QObject>

class WPacketSerialPortIo : public WPacketIoT<WSerialPort>
{
    Q_OBJECT
public:
    WPacketSerialPortIo(WPacketModel* packetModel, QObject* parent=0);
    virtual ~WPacketSerialPortIo();
	bool open();
	void close();
	WSerialPort* port() {
		return WPacketIoT<WSerialPort>::io();
	}
};
