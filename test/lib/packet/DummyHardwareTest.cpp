
#include "DummyHardwareTest.h"
#include "common/global/WAutoTest.h"

DummyHardwareTest::DummyHardwareTest(QObject* parent)
	: QObject(parent)
{
}

//virtual
DummyHardwareTest::~DummyHardwareTest()
{
}

void DummyHardwareTest::initTestCase()
{
}

void DummyHardwareTest::cleanupTestCase()
{
}

void DummyHardwareTest::init()
{
	m_hw = new DummyHardware;
}

void DummyHardwareTest::cleanup()
{
	delete m_hw;
	m_hw = 0;
}



DECLARE_TEST(DummyHardwareTest);
