#pragma once

#include "WPacketModel.h"

class WPacketHeadTailModel : public WPacketModel
{
	enum State
	{
		INIT,
		HEAD_DETECTED,
	};
public:
	WPacketHeadTailModel(const QByteArray& head,
						 const QByteArray& tail,
						 QObject* parent=0);
	virtual QByteArray beginRead(WPacketIo* packetIo);
	virtual void endRead();
	virtual QByteArray beginWrite(WPacket& packet);
	virtual void endWrite();
	virtual bool canReceiveBlock(WPacketIo* packetIo);
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	QByteArray m_head;
	QByteArray m_tail;
	State m_state;
	QByteArray m_body;
};
