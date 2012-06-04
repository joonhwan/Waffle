#pragma once

#include "packet/WPacketIoT.h"
#include <QTcpSocket>

class WTcpSocket : public WPacketIoT<QTcpSocket>
{
	Q_OBJECT
public:
	WTcpSocket(WPacketModel* packetModel, QObject* parent);
	void connectToHost(const QString& address, quint16 port,
					   QIODevice::OpenMode openMode=QIODevice::ReadWrite);
	void connectToHost(const QHostAddress& address, quint16 port,
					   QIODevice::OpenMode openMode=QIODevice::ReadWrite);
	void close();
	bool isOpen() const;
};

