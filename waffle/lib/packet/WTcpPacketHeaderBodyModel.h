#pragma once

#include "WTcpPacketModel.h"

class WTcpPacketHeaderBodyModel : public WTcpPacketModel
{
	Q_OBJECT
public:
	WTcpPacketHeaderBodyModel(QObject* parent=0);
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
	virtual bool canReceiveBlock(WTcpSocket* socket);
	virtual QByteArray beginRead(WTcpSocket* socket);
	virtual void endRead();
	virtual QByteArray beginWrite(WTcpSocketPacket& packet);
	virtual void endWrite();
    virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
protected:
	State m_state;
	HeaderData m_nextBlockHeader;
};
