#include "WTcpSocket.h"
#include "WTcpPacketModel.h"

#include <QByteArray>
#include <QHostAddress>
#include <QTime>

WTcpSocket::WTcpSocket(WTcpPacketModel* packetModel,
					   QObject* parent)
	: QTcpSocket(parent)
	, m_packetModel(packetModel)
{
	connect(this, SIGNAL(disconnected()), SLOT(onDisconnectedByServer()));
	connect(this, SIGNAL(readyRead()), SLOT(tryReceive()));
}

//virtual
WTcpSocket::~WTcpSocket()
{
}

void WTcpSocket::onDisconnectedByServer()
{
	this->close();
}

void WTcpSocket::tryReceive()
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

bool WTcpSocket::tryReceiveOneBlock(quint32& packetId, QByteArray& block)
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

bool WTcpSocket::send(WTcpSocketPacket& packet)
{
	bool sent = false;

	do {
		WTcpPacketModelScopedWrite lock(packetModel(), packet);

		QByteArray packetBlock = lock.blockToWrite();
		if (packetBlock.isNull()) {
			emit error(tr("null block framing error."));
			break;
		}

		if(-1 == this->write(packetBlock)) {
			emit error(tr("unable to send packet"));
			break;
		}

		sent = true;
	} while (0); // 1-pass loop

	return sent;
}

bool WTcpSocket::receive(quint32& packetId, QByteArray& block, int msecs)
{
	const int RETRY = 5;
	int waitInterval = (msecs > 500) ?  (msecs / RETRY) : msecs;
	QTime time;
	time.start();
	block = QByteArray(); // reset
	Q_ASSERT(block.isNull());
	while (time.elapsed() < msecs) {
		if (!waitForReadyRead(waitInterval)) {
			continue;
		}

		if (tryReceiveOneBlock(packetId, block)) {
			break;
		}
	}
	return !block.isNull();
}
