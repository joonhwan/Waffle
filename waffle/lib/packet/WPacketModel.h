#pragma once

#include <QObject>
#include <QByteArray>

class WPacket;
class WPacketIo;

class WPacketModel : public QObject
{
public:
	WPacketModel(QObject* parent=0)
		: QObject(parent)
	{
	}
	virtual QByteArray beginRead(WPacketIo* packetIo) = 0;
	virtual void endRead() = 0;
	virtual QByteArray beginWrite(WPacket& packet) = 0;
	virtual void endWrite() = 0;
	virtual bool canReceiveBlock(WPacketIo* packetIo) = 0;
    virtual quint32 nextBlockId() const = 0;
	virtual quint32 nextBlockLength() const = 0;
	QByteArray frame(WPacket& packet) {
		return beginWrite(packet);
	}
};

class WTcpPacketModelScopedRead
{
public:
	WTcpPacketModelScopedRead(WPacketModel* model, WPacketIo* packetIo)
		: m_model(model)
	{
		m_block = m_model->beginRead(packetIo);
	}
	virtual ~WTcpPacketModelScopedRead()
	{
		m_model->endRead();
	}
	QByteArray& block() {
		return m_block;
	}
	const QByteArray& block() const {
		return m_block;
	}
protected:
	WPacketModel* m_model;
	QByteArray m_block;
};

class WTcpPacketModelScopedWrite
{
public:
	WTcpPacketModelScopedWrite(WPacketModel* model, WPacket& packet)
		: m_model(model)
	{
		 m_block = m_model->beginWrite(packet);
	}
	virtual ~WTcpPacketModelScopedWrite()
	{
		m_model->endRead();
	}
	QByteArray& blockToWrite() {
		return m_block;
	}
protected:
	WPacketModel* m_model;
	QByteArray m_block;
};
