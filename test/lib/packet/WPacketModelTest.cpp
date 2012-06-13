#include "WPacketModelTest.h"
#include "Protocol.h"
#include "mock/WIODeviceMock.h"
#include "packet/WPacketIoT.h"
#include "common/global/WAutoTest.h"

const int LONG_TIME = 100000;

WPacketModelTest::WPacketModelTest(QObject* parent)
	: QObject(parent)
	, io(0)
{
}

//virtual
WPacketModelTest::~WPacketModelTest()
{
}

void WPacketModelTest::initTestCase()
{
}

void WPacketModelTest::cleanupTestCase()
{
}

void WPacketModelTest::init()
{
	io = new WIODeviceMock;
	io->open(QIODevice::ReadWrite);
}

void WPacketModelTest::cleanup()
{
	delete io;
}

void WPacketModelTest::packetSerializeTest()
{
	// packet model
	DummyHardwarePacketModel model;

	// a packet
	AvatecLedSetBrightnessPacket packet;
	packet.command = "HELL";
	packet.brightness = 18;

	// write test
	QByteArray bytes = model.frame(packet);
	int expectedLen = 0
					  + sizeof (char) // stx
					  + packet.command.fixedLength()
					  + packet.brightness.fixedLength()
					  + sizeof (short) // check sum
					  + sizeof (char) // etx
					  ;
	QCOMPARE(bytes.length(),expectedLen);
	int i = 0;
	QCOMPARE(bytes.at(i++),'\x02');
	QCOMPARE(bytes.at(i++),'H');
	QCOMPARE(bytes.at(i++),'E');
	QCOMPARE(bytes.at(i++),'L');
	QCOMPARE(bytes.at(i++),'L');
	QCOMPARE(bytes.at(i++),'0');
	QCOMPARE(bytes.at(i++),'1');
	QCOMPARE(bytes.at(i++),'8');
	i+=sizeof (short); // skip check sum
	QCOMPARE(bytes.at(i++),'\x0d');

	//  read test
	WPacketIoT<WIODeviceMock> mockIo(&model, this);
	mockIo.io()->appendInput(bytes);
	QByteArray readBytes;
	// QVERIFY(model.canReceiveBlock(&mockIo));
	QVERIFY(mockIo.receive(readBytes, LONG_TIME));
	QCOMPARE(readBytes, bytes);

	// // parse test
	// AvatecLedSetBrightnessPacket packetB;
	// {
	// 	WPacketStream s(bytes, Wf::StreamToData);
	// 	packetB.serialize(s,0);
	// }
	// QVERIFY(packet.brightness==packetB.brightness);
	// // QVERIFY(packet==packetB);
}

void WPacketModelTest::headTailModelTest()
{
}

DECLARE_TEST(WPacketModelTest);
