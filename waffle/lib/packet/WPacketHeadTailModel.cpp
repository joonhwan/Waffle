#include "WPacketHeadTailModel.h"
#include "WPacket.h"

WPacketHeadTailModel::WPacketHeadTailModel(const QByteArray& head,
										   const QByteArray& tail,
										   QObject* parent)
	: WPacketModel(parent)
	, m_head(head)
	, m_tail(tail)
	, m_state(INIT)
{
}

//virtual
QByteArray WPacketHeadTailModel::beginRead(WPacketIo* packetIo)
{
	Q_UNUSED(packetIo);
	return m_body; // or... m_head + m_body + m_tail ?
}

//virtual
void WPacketHeadTailModel::endRead()
{
	m_state = INIT;
	m_body.clear();
}

//virtual
QByteArray WPacketHeadTailModel::beginWrite(WPacket& packet)
{
	QByteArray block = m_head;
	block.append(packet.toBlock());
	block.append(m_tail);
	return block;
}

//virtual
void WPacketHeadTailModel::endWrite()
{
}

//virtual
bool WPacketHeadTailModel::canReceiveBlock(WPacketIo* packetIo)
{
	bool keepTry = true;
	int headLen = m_head.size();
	int tailLen = m_tail.size();

	Q_ASSERT(headLen>0 && tailLen>0); // ??? check your protocol spec!
	if(headLen==0 || tailLen==0) {
		return false;
	}

	while (keepTry) {
		keepTry = false;
		switch (m_state) {
		case INIT:
			// trying to read header till buffer ran out.
			while (packetIo->bytesAvailable() >= headLen) {
				QByteArray block = packetIo->peek(headLen);
				if (block==m_head) {
					// drop header bytes
					packetIo->rawReceive(headLen);
					// begin to read body
					m_state = HEAD_DETECTED;
					m_body.clear(); // for sure
					keepTry = true;
					break;
				} else {
					char dropByte;
					packetIo->rawReceive(&dropByte, 1);
					continue;
				}
			}
			break;
		case HEAD_DETECTED:
			while (packetIo->bytesAvailable() >= tailLen) {
				QByteArray block = packetIo->peek(tailLen);
				if (block==m_tail) {
					m_state = INIT;
					// drop tail bytes
					packetIo->rawReceive(tailLen);
					break;
				} else {
					// read body's 1-byte
					char bodyByte;
					packetIo->rawReceive(&bodyByte, 1);
					m_body.append(bodyByte);
					continue;
				}
			}
			break;
		case TAIL_DETECTED:
			// nothing to do
			break;
		}
	}
	return m_state==TAIL_DETECTED;
}

//virtual
quint32 WPacketHeadTailModel::nextBlockId() const
{
	// not important...
	return 0;
}

//virtual
quint32 WPacketHeadTailModel::nextBlockLength() const
{
	return (quint32)m_body.size();
}
