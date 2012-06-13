#pragma once

#include <QIODevice>

class WIODeviceMock : public QIODevice
{
	Q_OBJECT
public:
	WIODeviceMock(QObject* object = 0);
	virtual ~WIODeviceMock();

	virtual bool open(OpenMode mode);
	virtual qint64 bytesAvailable() const;
	virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);
	virtual bool isSequential() const;
	void appendInput(const QByteArray& inputToAdd);
	const QByteArray& output() const;
signals:
	void opened();
protected:
	virtual qint64 readData(char* data, qint64 maxSize);
	virtual qint64 writeData(const char* data, qint64 maxSize);
private:
	bool recvNextChunk();
	QByteArray m_input;
	QByteArray m_output;
};
