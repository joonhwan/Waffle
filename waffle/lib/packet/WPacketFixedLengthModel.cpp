#include "WPacketFixedLengthModel.h"
#include "WPacketIo.h"
#include "WPacket.h"

WPacketFixedLengthModel::WPacketFixedLengthModel(int fixedPacketLength, QObject* parent)
	: WPacketModel(parent)
	, m_fixedPacketLength(fixedPacketLength)
{
}

//virtual
QByteArray WPacketFixedLengthModel::beginRead(WPacketIo* packetIo)
{
	return packetIo->rawReceive(m_fixedPacketLength);
}

//virtual
void WPacketFixedLengthModel::endRead()
{
}

//virtual
QByteArray WPacketFixedLengthModel::beginWrite(WPacket& packet)
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
void WPacketFixedLengthModel::endWrite()
{
}

//virtual
bool WPacketFixedLengthModel::canReceiveBlock(WPacketIo* packetIo)
{
	return packetIo->bytesAvailable() > m_fixedPacketLength;
}

//virtual
quint32 WPacketFixedLengthModel::nextBlockId() const
{
	return 0;
}

//virtual
quint32 WPacketFixedLengthModel::nextBlockLength() const
{
	return m_fixedPacketLength;
}
