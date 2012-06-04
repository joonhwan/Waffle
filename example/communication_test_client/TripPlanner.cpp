#include "TripPlanner.h"
#include "packet/WTcpSocket.h"
#include "packet/WTcpPacketHeaderBodyModel.h"
#include <QByteArray>
#include <QHostAddress>

TripPlanner::TripPlanner(QObject* parent)
	: QObject(parent)
{
	m_socket = new WTcpSocket(new WTcpPacketHeaderBodyModel(this), this);
	connect(m_socket, SIGNAL(connected()), SLOT(sendRequest()));
	connect(m_socket, SIGNAL(disconnected()), SLOT(onDisconnectedByServer()));

	connect(m_socket,
			SIGNAL(readyBlock(quint32, const QByteArray&)),
			SLOT(onReceive(quint32, const QByteArray&)));
}

//virtual
TripPlanner::~TripPlanner()
{
}

void TripPlanner::setQuery(TripPlanQueryPacket& query)
{
	m_query = query;
}

void TripPlanner::search(TripPlanQueryPacket& query)
{
	setQuery(query);
	search();
}

void TripPlanner::search()
{
	m_nextBlockSize = 0;
	m_socket->connectToHost(QHostAddress::LocalHost, TRIP_SERVER_PORT);
	emit stateChanged(tr("connecting to server..."));
}

void TripPlanner::stopSearch()
{
	if (m_socket->isOpen()) {
		m_socket->close();
		emit stateChanged(tr("search ended."));
		emit stoppedSearch();
	}
}

void TripPlanner::sendRequest()
{
	emit stateChanged(tr("sending request..."));
	m_socket->send(m_query);
}

void TripPlanner::onDisconnectedByServer()
{
	m_socket->close();
}

void TripPlanner::onReceive(quint32 packetId, const QByteArray& block)
{
	QDataStream in(block);
	switch (packetId)
	{
	case TripPlanProtocol::RESULT:
		{
			TripPlanResultPacket resultPacket;
			if (resultPacket.fromBlock(block)) {
				emit received(resultPacket);
			}
		}
		break;
	case TripPlanProtocol::COMMAND:
		{
			TripPlanCommandPacket commandPacket;
			if (commandPacket.fromBlock(block)) {
				if (commandPacket.command == "EOF") {
					stopSearch();
				}
			}
		}
		break;
	}
}
