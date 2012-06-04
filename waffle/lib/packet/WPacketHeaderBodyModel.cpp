#include "WPacketHeaderBodyModel.h"
#include "WPacketIo.h"
#include "WPacket.h"

namespace {

const quint32 WTCPSOCKET_MAGIC = 0x50435457; // 'WTCP'

}

WPacketHeaderBodyModel::WPacketHeaderBodyModel(QObject* parent)
	: WPacketModel(parent)
{
	endRead();
}

//virtual
bool WPacketHeaderBodyModel::canReceiveBlock(WPacketIo* packetIo)
{
	const int headerSize = sizeof (HeaderData);
	bool keepTry = true;
	while (keepTry) {
		keepTry = false;
		switch (m_state) {
		case INIT:
			// trying to read header till buffer ran out.
			while (packetIo->bytesAvailable() >= headerSize) {
				quint32 magic = 0;
				packetIo->peek((char*)&magic, sizeof (magic));
				Q_ASSERT (magic == WTCPSOCKET_MAGIC);
				if (magic != WTCPSOCKET_MAGIC) {
					char dropByte;
					// 1 byte skip by reading out
					packetIo->rawReceive(&dropByte, 1);
					// try next time..
					continue;
				}

				packetIo->rawReceive((char*)&m_nextBlockHeader, headerSize);
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
			if (packetIo->bytesAvailable() >= nextBlockLength()) {
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
QByteArray WPacketHeaderBodyModel::beginRead(WPacketIo* packetIo)
{
	// nothing to do
	int len = nextBlockLength();
	if (len >= 0) {
		return packetIo->rawReceive(nextBlockLength());
	} else {
		return QByteArray();
	}
}

//virtual
void WPacketHeaderBodyModel::endRead()
{
	// reset internal intermediate header/state
	m_state = INIT;
	::memset(&m_nextBlockHeader, 0, sizeof (m_nextBlockHeader));
}

QByteArray WPacketHeaderBodyModel::beginWrite(WPacket& packet)
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
void WPacketHeaderBodyModel::endWrite()
{
	// nothing to do
}

// virtual
quint32 WPacketHeaderBodyModel::nextBlockId() const
{
	return m_nextBlockHeader.id;
}

//virtual
quint32 WPacketHeaderBodyModel::nextBlockLength() const
{
	return m_nextBlockHeader.bodyLen;
}
