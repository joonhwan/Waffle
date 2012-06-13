#pragma once

#include "packet/WPacketTcpIo.h"
#include <QByteArray>

class TripClient : public WPacketTcpIo
{
    Q_OBJECT
public:
    TripClient(QObject* parent=0);
    virtual ~TripClient();
private slots:
	void onReceive(quint32 packetId, QByteArray& block);
};
