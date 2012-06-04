#pragma once

#include "packet/WTcpSocket.h"
#include <QByteArray>

class TripClient : public WTcpSocket
{
    Q_OBJECT
public:
    TripClient(QObject* parent=0);
    virtual ~TripClient();
private slots:
	void onReceive(quint32 packetId, const QByteArray& block);
};
