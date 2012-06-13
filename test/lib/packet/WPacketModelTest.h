#pragma once

#include <QObject>

class WIODeviceMock;

class WPacketModelTest : public QObject
{
    Q_OBJECT
public:
    WPacketModelTest(QObject* parent=0);
    virtual ~WPacketModelTest();
private slots:
	// suite init/cleanup
	void initTestCase();
	void cleanupTestCase();
	// per-test init/cleanup
	void init();
	void cleanup();
	// test case follows.
	void packetSerializeTest();
	void headTailModelTest();
protected:
	WIODeviceMock* io;
};
