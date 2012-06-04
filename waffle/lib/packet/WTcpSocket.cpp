#include "WTcpSocket.h"

WTcpSocket::WTcpSocket(WPacketModel* packetModel,
					   QObject* parent)
	: WPacketIoT<QTcpSocket>(packetModel, parent)
{
}

void WTcpSocket::connectToHost(const QString& address, quint16 port,
							   QIODevice::OpenMode openMode)
{
	io()->connectToHost(address, port, openMode);
}

void WTcpSocket::connectToHost(const QHostAddress& address, quint16 port,
							   QIODevice::OpenMode openMode)
{
	io()->connectToHost(address, port, openMode);
}

void WTcpSocket::close()
{
	io()->close();
}

bool WTcpSocket::isOpen() const
{
	return io()->isOpen();
}

