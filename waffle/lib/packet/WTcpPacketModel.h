#pragma once

#include <QObject>

class WTcpSocketPacket;
class WTcpSocket;

class WTcpPacketModel : public QObject
{
public:
	WTcpPacketModel(QObject* parent=0)
		: QObject(parent)
	{
	}
	virtual QByteArray beginRead(WTcpSocket* socket) = 0;
	virtual void endRead() = 0;
	virtual QByteArray beginWrite(WTcpSocketPacket& packet) = 0;
	virtual void endWrite() = 0;
	virtual bool canReceiveBlock(WTcpSocket* socket) = 0;
    virtual quint32 nextBlockId() const = 0;
	virtual quint32 nextBlockLength() const = 0;
};

class WTcpPacketModelScopedRead
{
public:
	WTcpPacketModelScopedRead(WTcpPacketModel* model, WTcpSocket* socket)
		: m_model(model)
	{
		m_block = m_model->beginRead(socket);
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
	WTcpPacketModel* m_model;
	QByteArray m_block;
};

class WTcpPacketModelScopedWrite
{
public:
	WTcpPacketModelScopedWrite(WTcpPacketModel* model, WTcpSocketPacket& packet)
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
	WTcpPacketModel* m_model;
	QByteArray m_block;
};
