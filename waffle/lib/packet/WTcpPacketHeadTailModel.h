#pragma once

#include "WTcpPacketModel.h"

class WTcpPacketHeadTailModel : public WTcpPacketModel
{
	enum State
	{
		INIT,
		HEAD_DETECTED,
		TAIL_DETECTED,
	};
public:
	WTcpPacketHeadTailModel(const QByteArray& head,
							const QByteArray& tail,
							QObject* parent=0)
	virtual void beginRead(WTcpSocket* socket);
	virtual void endRead();
	virtual QByteArray beginWrite(WTcpSocketPacket& packet);
	virtual void endWrite();
	virtual bool canReceiveBlock(WTcpSocket* socket);
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	QByteArray m_head;
	QByteArray m_tail;
	State m_state;
	QByteArray m_body;
};
