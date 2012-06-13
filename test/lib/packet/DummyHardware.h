#pragma once

#include <QObject>

class WPacketSerialPortIo;

class DummyHardware : public QObject
{
	Q_OBJECT
public:
    DummyHardware(QObject* parent=0);
    virtual ~DummyHardware();

	bool openHardware();
	void closeHardware();
	bool powerOn(bool on=true);
	bool setBrightness(int value);
private slots:
	void onReceive(quint32 packetId, const QByteArray& block);
protected:
	WPacketSerialPortIo* m_port;
};
