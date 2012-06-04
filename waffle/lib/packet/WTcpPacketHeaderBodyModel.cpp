#include "WTcpPacketHeaderBodyModel.h"
#include "WTcpSocket.h"
#include "WTcpSocketPacket.h"

namespace {

const quint32 WTCPSOCKET_MAGIC = 0x50435457; // 'WTCP'

}

WTcpPacketHeaderBodyModel::WTcpPacketHeaderBodyModel(QObject* parent)
	: WTcpPacketModel(parent)
{
	endRead();
}

//virtual
bool WTcpPacketHeaderBodyModel::canReceiveBlock(WTcpSocket* socket)
{
	const int headerSize = sizeof (HeaderData);
	bool keepTry = true;
	while (keepTry) {
		keepTry = false;
		switch (m_state) {
		case INIT:
			// trying to read header till buffer ran out.
			while (socket->bytesAvailable() >= headerSize) {
				quint32 magic = 0;
				socket->peek((char*)&magic, sizeof (magic));
				Q_ASSERT (magic == WTCPSOCKET_MAGIC);
				if (magic != WTCPSOCKET_MAGIC) {
					char dropByte;
					// 1 byte skip by reading out
					socket->read(&dropByte, 1);
					// try next time..
					continue;
				}

				socket->read((char*)&m_nextBlockHeader, headerSize);
				keepTry = true; // try to get more(=body)

				// what if zero length block is detected? :(
				// zerobyte block should be skipped by reading out header bytes. :(
				Q_ASSERT(nextBlockLength() > 0);

				if (nextBlockLength() > 0) {
					m_state = RECEIVED_HEADER;
					break;
				}
			}
			break;
		case RECEIVED_HEADER:
			if (socket->bytesAvailable() >= nextBlockLength()) {
				m_state = RECEIVED_BODY;
			}
			break;
		case RECEIVED_BODY:
			// nothing to do
			break;
		}
	}

	return (m_state == RECEIVED_BODY);
}

// virtual
QByteArray WTcpPacketHeaderBodyModel::beginRead(WTcpSocket* socket)
{
	// nothing to do
	int len = nextBlockLength();
	if (len >= 0) {
		return socket->read(nextBlockLength());
	} else {
		return QByteArray();
	}
}

//virtual
void WTcpPacketHeaderBodyModel::endRead()
{
	// reset internal intermediate header/state
	m_state = INIT;
	::memset(&m_nextBlockHeader, 0, sizeof (m_nextBlockHeader));
}

QByteArray WTcpPacketHeaderBodyModel::beginWrite(WTcpSocketPacket& packet)
{
	const int headerLen = sizeof (HeaderData);
	QByteArray block(headerLen, 0); // byte array holding header
	QByteArray packetBlock = packet.toBlock();
	block.append(packetBlock); // append packetBlock after header

	HeaderData* header = (HeaderData*)block.data();
	::memset(header, 0, headerLen);
	header->magic = WTCPSOCKET_MAGIC;
	header->bodyLen = block.size() - headerLen;
	header->id = packet.id();

	// block = header + body.
	return block;
}

//virtual
void WTcpPacketHeaderBodyModel::endWrite()
{
	// nothing to do
}

// virtual
quint32 WTcpPacketHeaderBodyModel::nextBlockId() const
{
	return m_nextBlockHeader.id;
}

//virtual
quint32 WTcpPacketHeaderBodyModel::nextBlockLength() const
{
	return m_nextBlockHeader.bodyLen;
}
