#pragma once

#include <QObject>

class DummyHardwareTest : public QObject
{
    Q_OBJECT
public:
    DummyHardwareTest(QObject* parent=0);
    virtual ~DummyHardwareTest();
private slots:
	// suite init/cleanup
	void initTestCase();
	void cleanupTestCase();
	// per-test init/cleanup
	void init();
	void cleanup();
	// test case follows.

protected:
	DummyHardware m_hw;
};

