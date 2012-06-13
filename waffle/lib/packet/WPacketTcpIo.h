#pragma once

#include "packet/WPacketIoT.h"
#include <QTcpSocket>

class WPacketTcpIo : public WPacketIoT<QTcpSocket>
{
	Q_OBJECT
public:
	WPacketTcpIo(WPacketModel* packetModel, QObject* parent);
	void connectToHost(const QString& address, quint16 port,
					   QIODevice::OpenMode openMode=QIODevice::ReadWrite);
	void connectToHost(const QHostAddress& address, quint16 port,
					   QIODevice::OpenMode openMode=QIODevice::ReadWrite);
	void close();
	bool isOpen() const;
	QTcpSocket* socket() {
		return WPacketIoT<QTcpSocket>::io();
	}
private slots:
	void onDisconnectedByServer();
};

