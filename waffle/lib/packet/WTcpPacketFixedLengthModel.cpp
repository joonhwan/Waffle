#include "WTcpPacketFixedLengthModel.h"

WTcpPacketFixedLengthModel::WTcpPacketFixedLengthModel(int fixedPacketLength, QObject* parent)
	: WTcpPacketModel(parent)
	, m_fixedPacketLength(fixedPacketLength)
{
}

//virtual
QByteArray WTcpPacketFixedLengthModel::beginRead(WTcpSocket* socket)
{
	return this->read(m_fixedPacketLength)
}

//virtual
void WTcpPacketFixedLengthModel::endRead()
{
}

//virtual
QByteArray WTcpPacketFixedLengthModel::beginWrite(WTcpSocketPacket& packet)
{
	QByteArray block = packet.toBlock();
	int margin = m_fixedPacketLength - block.size();
	Q_ASSERT (margin >= 0);
	if (margin >= 0) {
		block.append(QByteArray(margin,0));
	} else {
		//block.resize();
		block = QByteArray(); // nullify
	}
	return block;
}

//virtual
void WTcpPacketFixedLengthModel::endWrite()
{
}

//virtual
bool WTcpPacketFixedLengthModel::canReceiveBlock(WTcpSocket* socket)
{
	return socket->bytesAvailable() > m_fixedPacketLength;
}

//virtual
quint32 WTcpPacketFixedLengthModel::nextBlockId() const
{
	return 0;
}

//virtual
quint32 WTcpPacketFixedLengthModel::nextBlockLength() const
{
	return m_fixedPacketLength;
}
