#include "WPacketSerialPortIo.h"

WPacketSerialPortIo::WPacketSerialPortIo(WPacketModel* packetModel,
										 QObject* parent)
	: WPacketIoT<WSerialPort>(packetModel, parent)
{
}

//virtual
WPacketSerialPortIo::~WPacketSerialPortIo()
{
}

bool WPacketSerialPortIo::open()
{
	if (!io()->isOpen()) {
		io()->open(QIODevice::ReadWrite);
	}
	return io()->isOpen();
}

void WPacketSerialPortIo::close()
{
	io()->close();
}
