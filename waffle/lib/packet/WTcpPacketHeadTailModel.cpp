#include "WTcpPacketHeadTailModel.h"

WTcpPacketHeadTailModel::WTcpPacketHeadTailModel(int fixedPacketLength, QObject* parent)
	: WTcpPacketModel(parent)
	, m_fixedPacketLength(fixedPacketLength)
	, m_state(INIT)
{
}

//virtual
QByteArray WTcpPacketHeadTailModel::beginRead(WTcpSocket* socket)
{
	Q_UNUSED(socket);
	return m_body; // or... m_head + m_body + m_tail ?
}

//virtual
void WTcpPacketHeadTailModel::endRead()
{
	m_state = INIT;
	m_body.clear();
}

//virtual
QByteArray WTcpPacketHeadTailModel::beginWrite(WTcpSocketPacket& packet)
{
	QByteArray block = m_head;
	block.append(packet.toBlock());
	block.append(m_tail);
	return block;
}

//virtual
void WTcpPacketHeadTailModel::endWrite()
{
}

//virtual
bool WTcpPacketHeadTailModel::canReceiveBlock(WTcpSocket* socket)
{
	bool keepTry = true;
	int headLen = m_head.size();
	int tailLen = m_tail.size();
	Q_ASSERT( tailLen != 0); // ??? check your protocol spec!

	while (keepTry) {
		keepTry = false;
		switch (m_state) {
		case INIT:
			// trying to read header till buffer ran out.
			while (socket->bytesAvailable() >= headLen) {
				if (0==headLen) {
					keepTry = true;
					m_state = HEAD_DETECTED;
					break;
				}
				QByteArray block = socket->peek(headLen);
				if (block==m_head) {
					m_state = HEAD_DETECTED;
					m_body.clear(); // for sure
					keepTry = true;
					break;
				} else {
					char dropByte;
					socket->read(&dropByte, 1);
					continue;
				}
			}
			break;
		case HEAD_DETECTED:
			while (socket->bytesAvailable() >= tailLen) {
				if (0==tailLen) {
					m_body = socket->read(socket->bytesAvailable());
					m_state = TAIL_DETECTED;
					break;
				}
				QByteArray block = socket->peek(tailLen);
				if (block==m_tail) {
					m_state = TAIL_DETECTED;
					break;
				} else {
					char bodyByte;
					socket->read(&bodyByte, 1);
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
quint32 WTcpPacketHeadTailModel::nextBlockId() const
{
	// not important...
	return 0;
}

//virtual
quint32 WTcpPacketHeadTailModel::nextBlockLength() const
{
	return m_nextBlockLengthBeforeTail;
}
