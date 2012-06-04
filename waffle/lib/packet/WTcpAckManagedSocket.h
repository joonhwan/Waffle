#pragma once

#include <WTcpSocket>

class WTcpAckManagedSocket : public WTcpSocket
{
    Q_OBJECT
public:
    WTcpAckManagedSocket(QObject* parent=0);
    virtual ~WTcpAckManagedSocket();
signals:
public slots:
protected:
};
