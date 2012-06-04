#pragma once

#include "WTcpPacketModel.h"

class WTcpPacketFixedLengthModel : public WTcpPacketModel
{
public:
	WTcpPacketFixedLengthModel(int fixedPacketLength, QObject* parent=0)
	virtual QByteArray beginRead(WTcpSocket* socket);
	virtual void endRead();
	virtual QByteArray beginWrite(WTcpSocketPacket& packet);
	virtual void endWrite();
	virtual bool canReceiveBlock(WTcpSocket* socket);
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	int m_fixedPacketLength;
};

template<int PACKET_LENGTH>
class WTcpPacketFixedLengthModelT : public WTcpPacketFixedLengthModel
{
public:
	WTcpPacketFixedLengthModel(QObject* parent=0)
		: WTcpPacketFixedLengthModel(PACKET_LENGTH, parent)
	{
	}
};

