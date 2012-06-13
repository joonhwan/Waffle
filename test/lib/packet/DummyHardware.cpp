#include "DummyHardware.h"
#include "Protocol.h"
#include "packet/WPacket.h"
#include "packet/WPacketHeadTailModel.h"
#include "packet/WPacketSerialPortIo.h"

namespace {
}

DummyHardware::DummyHardware(QObject* parent)
	: QObject(parent)
{
	DummyHardwarePacketModel* packetModel = new DummyHardwarePacketModel(this);
	m_port = new WPacketSerialPortIo(packetModel, this);
	m_port->io()->setQueryMode(WSerialPort::Polling);

	// connect(m_port->io(), SIGNAL(readyBlock(quint32, const QByteArray&)));
}

//virtual
DummyHardware::~DummyHardware()
{
}

bool DummyHardware::openHardware()
{
	return m_port->open();
}

void DummyHardware::closeHardware()
{
	m_port->close();
}

bool DummyHardware::powerOn(bool on)
{
	bool done = false;

	do
	{
		AvatecLedCommandPacket packet;
		packet.command = on ? "OPON" : "OPOF";

		if (!m_port->send(packet)) {
			break;
		}

		QByteArray response;
		if (!m_port->receive(response)) {
			break;
		}

		done = (response[0]=='\x06');
	} while (0);

	return done;
}

bool DummyHardware::setBrightness(int value)
{
	bool done = false;

	do
	{
		AvatecLedSetBrightnessPacket packet;
		packet.command = "SETI";
		packet.brightness = value;

		if (!m_port->send(packet)) {
			break;
		}

		QByteArray response;
		if (!m_port->receive(response)) {
			break;
		}

		done = (response[0]=='\x06');
	} while (0);

	return done;
}

void DummyHardware::onReceive(quint32 packetId, const QByteArray& block)
{
}

