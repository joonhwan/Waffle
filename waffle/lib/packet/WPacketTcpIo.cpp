#include "WPacketTcpIo.h"

WPacketTcpIo::WPacketTcpIo(WPacketModel* packetModel,
						   QObject* parent)
	: WPacketIoT<QTcpSocket>(packetModel, parent)
{
	Q_ASSERT(io());
	connect(io(), SIGNAL(disconnected()), SLOT(onDisconnectedByServer()));
}

void WPacketTcpIo::connectToHost(const QString& address, quint16 port,
								 QIODevice::OpenMode openMode)
{
	io()->connectToHost(address, port, openMode);
}

void WPacketTcpIo::connectToHost(const QHostAddress& address, quint16 port,
								 QIODevice::OpenMode openMode)
{
	io()->connectToHost(address, port, openMode);
}

void WPacketTcpIo::close()
{
	io()->close();
}

bool WPacketTcpIo::isOpen() const
{
	return io()->isOpen();
}

void WPacketTcpIo::onDisconnectedByServer()
{
	Q_ASSERT(io());
	if (io()) {
		io()->close();
	}
}
