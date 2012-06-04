#pragma once

#include <QTcpServer>

class TripServer : public QTcpServer
{
    Q_OBJECT
public:
    TripServer(QObject* parent=0);
    virtual ~TripServer();
signals:
public slots:
protected:
	virtual void incomingConnection(int s);
};
