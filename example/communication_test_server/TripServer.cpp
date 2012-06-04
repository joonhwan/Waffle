#include "TripServer.h"
#include "TripClient.h"

TripServer::TripServer(QObject* parent)
	: QTcpServer(parent)
{
}

//virtual
TripServer::~TripServer()
{
}

//virtual
void TripServer::incomingConnection(int s)
{
	TripClient* client = new TripClient(this);
	client->setSocketDescriptor(s);
	// client->onReceive();
}

