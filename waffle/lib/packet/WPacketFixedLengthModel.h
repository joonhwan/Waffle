#pragma once

#include "WPacketModel.h"

class WPacketFixedLengthModel : public WPacketModel
{
public:
	WPacketFixedLengthModel(int fixedPacketLength, QObject* parent=0);
	virtual QByteArray beginRead(WPacketIo* packetIo);
	virtual void endRead();
	virtual QByteArray beginWrite(WPacket& packet);
	virtual void endWrite();
	virtual bool canReceiveBlock(WPacketIo* packetIo);
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	int m_fixedPacketLength;
};

template<int PACKET_LENGTH>
class WTcpPacketFixedLengthModelT : public WPacketFixedLengthModel
{
public:
	WTcpPacketFixedLengthModelT(QObject* parent=0)
		: WPacketFixedLengthModel(PACKET_LENGTH, parent)
	{
	}
};

