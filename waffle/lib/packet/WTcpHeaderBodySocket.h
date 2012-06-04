#pragma once

#include <QtEndian>
#include <QTcpSocket>

class WPacket;

class WTcpHeaderBodySocket : public QTcpSocket
{
	Q_OBJECT
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
public:
    WTcpHeaderBodySocket(QObject* parent=0);
    virtual ~WTcpHeaderBodySocket();
	virtual bool canReceiveBlock();
	virtual bool send(WPacket& packet, quint32 id=0);
	virtual bool receive(WPacket& packet);
	virtual quint32 nextBlockId() const;
	virtual quint32 nextBlockLength() const;
signals:
	void error(const QString& reason);
private slots:
	void resetState();
protected:
	State m_state;
	HeaderData m_nextBlockHeader;
};

inline
quint32 WTcpHeaderBodySocket::nextBlockId() const
{
	return m_nextBlockHeader.id;
}

inline
quint32 WTcpHeaderBodySocket::nextBlockLength() const
{
	return m_nextBlockHeader.bodyLen;
}
