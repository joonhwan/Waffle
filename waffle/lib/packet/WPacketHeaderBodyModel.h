#pragma once

#include "WPacketModel.h"

class WPacketHeaderBodyModel : public WPacketModel
{
	Q_OBJECT
public:
	WPacketHeaderBodyModel(QObject* parent=0);
    // consider byte-align !
	struct HeaderData {
		quint32 magic;   // 4 byte
		quint32 bodyLen; // 4 byte
		quint32 id;		 // 4 byte
		quint32 dummy;
	};
	enum State {
		INIT,
		RECEIVED_HEADER,
		RECEIVED_BODY,
	};
	virtual bool canReceiveBlock(WPacketIo* packetIo);
	virtual QByteArray beginRead(WPacketIo* packetIo);
	virtual void endRead();
	virtual QByteArray beginWrite(WPacket& packet);
	virtual void endWrite();
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	State m_state;
	HeaderData m_nextBlockHeader;
};
