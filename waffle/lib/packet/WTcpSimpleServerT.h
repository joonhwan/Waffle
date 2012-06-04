#pragma once

#include <QTcpServer>

template<typename T>
class WTcpSimpleServerT : public QTcpServer
{
public:
    WTcpSimpleServerT(QObject* parent=0)
		: QTcpServer(parent)
	{
	}
protected:
	virtual void incomingConnection(int s)
	{
		T* client = new T(this);
		client->setSocketDescriptor(s);
		connect(connect, SIGNAL(disconnected()), SLOT(deleteLater()));
	}
};
