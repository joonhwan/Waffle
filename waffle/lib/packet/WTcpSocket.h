#pragma once

#include <QtEndian>
#include <QTcpSocket>

class WTcpSocketPacket;
class WTcpPacketModel;

class WTcpSocket : public QTcpSocket
{
	Q_OBJECT
public:
    WTcpSocket(WTcpPacketModel* packetModel,
			   QObject* parent=0);
    virtual ~WTcpSocket();
	bool send(WTcpSocketPacket& packet);
	bool receive(quint32& packetId, QByteArray& block, int msecs=1000);
	WTcpPacketModel* packetModel() {
		return m_packetModel;
	}
signals:
	void readyBlock(quint32 packetId, const QByteArray& block);
	void failedBlock(quint32 packetId);
	void error(const QString& reason);
private slots:
	void tryReceive();
	void onDisconnectedByServer();
protected:
	bool tryReceiveOneBlock(quint32& packetId, QByteArray& block);
private:
	WTcpPacketModel* m_packetModel;
};

template<typename ModleType>
class WTcpSocketT : public WTcpSocket
{
public:
	WTcpSocketT(QObject* parent)
		: WTcpSocket(new ModelType(parent),
					 parent)
	{
	}
};
