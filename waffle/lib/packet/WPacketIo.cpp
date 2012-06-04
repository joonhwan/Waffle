#include "WPacketIo.h"
#include "WPacketModel.h"

#include <QByteArray>
#include <QHostAddress>
#include <QTime>

WPacketIo::WPacketIo(WPacketModel* packetModel,
					   QIODevice* internalIo,
					   QObject* parent)
	: QObject(parent)
	, m_packetModel(packetModel)
	, m_io(internalIo)
{
	Q_ASSERT(m_io);
	connect(m_io, SIGNAL(disconnected()), SLOT(onDisconnectedByServer()));
	connect(m_io, SIGNAL(readyRead()), SLOT(tryReceive()));
}

//virtual
WPacketIo::~WPacketIo()
{
}

void WPacketIo::onDisconnectedByServer()
{
	Q_ASSERT(m_io);
	if (m_io) {
		m_io->close();
	}
}

void WPacketIo::tryReceive()
{
	quint32 packetId = 0;
	QByteArray block;

	forever {
		packetId = 0;
		if (tryReceiveOneBlock(packetId, block)) {
			if (block.isNull()) {
				// // how dare it can happen? (check your 'canReceiveBlockFrom' impl)
				// emit failedBlock(packetId);
			} else {
				emit readyBlock(packetId, block);
			}
		} else {
			break;
		}
	}
}

bool WPacketIo::tryReceiveOneBlock(quint32& packetId, QByteArray& block)
{
	bool received = false;

	do {
		if (!packetModel()->canReceiveBlock(this))
			break;

		WTcpPacketModelScopedRead lock(packetModel(), this);

		packetId = packetModel()->nextBlockId();
		int nextBlockLength = packetModel()->nextBlockLength();
		if (nextBlockLength < 0) {
			break;
		}

		if (nextBlockLength == 0) {
			// not null but zero-legnth..
			block.resize(0);
		} else {
			block = lock.block();
			Q_ASSERT(block.size() == nextBlockLength);
			if (block.size() != nextBlockLength) {
				break;
			}
		}

		received = true;

	} while (0); // 1-pass loop

	return received;
}

bool WPacketIo::send(WPacket& packet)
{
	bool sent = false;

	do {
		Q_ASSERT(m_io);
		if (!m_io) {
			break;
		}

		WTcpPacketModelScopedWrite lock(packetModel(), packet);

		QByteArray packetBlock = lock.blockToWrite();
		if (packetBlock.isNull()) {
			emit error(tr("null block framing error."));
			break;
		}

		if(-1 == m_io->write(packetBlock)) {
			emit error(tr("unable to send packet"));
			break;
		}

		sent = true;
	} while (0); // 1-pass loop

	return sent;
}

bool WPacketIo::receive(quint32& packetId, QByteArray& block, int msecs)
{
	Q_ASSERT(m_io);
	if (!m_io) {
		return false;
	}

	const int RETRY = 5;
	int waitInterval = (msecs > 500) ?  (msecs / RETRY) : msecs;
	QTime time;
	time.start();
	block = QByteArray(); // reset
	Q_ASSERT(block.isNull());
	while (time.elapsed() < msecs) {
		if (!m_io->waitForReadyRead(waitInterval)) {
			continue;
		}

		if (tryReceiveOneBlock(packetId, block)) {
			break;
		}
	}
	return !block.isNull();
}

QByteArray WPacketIo::rawReceive(qint64 maxSize)
{
	QByteArray ba;
	Q_ASSERT(m_io);
	if (m_io) {
		ba = m_io->read(maxSize);
	}
	return ba;
}

qint64 WPacketIo::rawReceive(char* data, qint64 maxSize)
{
	qint64 bytes = 0;
	Q_ASSERT(m_io);
	if (m_io) {
		bytes = m_io->read(data, maxSize);
	}
	return bytes;
}

qint64 WPacketIo::bytesAvailable() const
{
	qint64 bytes = 0;
	Q_ASSERT(m_io);
	if (m_io) {
		bytes = m_io->bytesAvailable();
	}
	return bytes;
}

QByteArray WPacketIo::peek(qint64 maxSize)
{
	QByteArray ba;
	Q_ASSERT(m_io);
	if (m_io) {
		ba = m_io->peek(maxSize);
	}
	return ba;
}

qint64 WPacketIo::peek(char* data, qint64 maxSize)
{
	qint64 bytes = 0;
	Q_ASSERT(m_io);
	if (m_io) {
		bytes = m_io->peek(data, maxSize);
	}
	return bytes;
}



