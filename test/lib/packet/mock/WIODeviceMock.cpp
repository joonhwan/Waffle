#include "WIODeviceMock.h"

WIODeviceMock::WIODeviceMock(QObject* object)
	: QIODevice(object)
{
	open(QIODevice::ReadWrite);
}

//virtual
WIODeviceMock::~WIODeviceMock()
{
}

//virtual
bool WIODeviceMock::open(OpenMode mode)
{
	return QIODevice::open(mode);
}

//virtual
qint64 WIODeviceMock::bytesAvailable() const
{
	return (qint64)m_input.length() + QIODevice::bytesAvailable();
}

//virtual
bool WIODeviceMock::waitForReadyRead(int msecs)
{
	return !m_input.isEmpty();
}

//virtual
bool WIODeviceMock::waitForBytesWritten(int msecs)
{
	return true;
}

//virtual
bool WIODeviceMock::isSequential() const
{
	return true;
}

void WIODeviceMock::appendInput(const QByteArray& inputToAdd)
{
	m_input.append(inputToAdd);
	// foreach(const char& c, inputToAdd) {
	// 	ungetChar(c);
	// }
}

const QByteArray& WIODeviceMock::output() const
{
	return m_output;
}

//virtual
qint64 WIODeviceMock::readData(char* data, qint64 maxSize)
{
	qint64 readByte = 0;
	if (openMode() & ReadOnly) {
		readByte = maxSize;
		if (readByte > (qint64)m_input.size()) {
			readByte = (qint64)m_input.size();
		}
		if (readByte) {
			memcpy(data, m_input.constData(), readByte);
			m_input.remove(0, readByte);
		}
	}
	return readByte;
}

//virtual
qint64 WIODeviceMock::writeData(const char* data, qint64 maxSize)
{
	qint64 writeByte = 0;
	if (openMode() & WriteOnly) {
		m_output.append(data, maxSize);
		writeByte =  (qint64)m_output.size();
	}
	return writeByte;
}

