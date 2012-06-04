#include "TripClient.h"
#include "Protocol.h"
#include "packet/WTcpPacketHeaderBodyModel.h"
#include <QDebug>

TripClient::TripClient(QObject* parent)
	: WTcpSocket(new WTcpPacketHeaderBodyModel(parent),
				 parent)
{
	connect(this,
			SIGNAL(readyBlock(quint32, const QByteArray&)),
			SLOT(onReceive(quint32, const QByteArray&)));
	connect(this, SIGNAL(disconnected()), SLOT(deleteLater()));
}

//virtual
TripClient::~TripClient()
{
}

void TripClient::onReceive(quint32 packetId, const QByteArray& block)
{
	switch (packetId)
	{
	case TripPlanProtocol::QUERY:
		{
			TripPlanQueryPacket query;
			if (query.fromBlock(block)) {
				qsrand(QDateTime::currentDateTime().toTime_t());

				int numTrips = qrand() % 1000;
				qDebug() << "generating " << numTrips << " results...";
				for (int i = 0; i < numTrips; ++i) {
					TripPlanResultPacket result;
					result.departure = query.time;
					result.duration = qrand() % 200;
					result.changes = 1;
					result.trainType = QString("InterCity");

					this->send(result);
				}

				{
					TripPlanCommandPacket commandPacket;
					commandPacket.command = "EOF";
					this->send(commandPacket);
				}
			}
		}
		break;
	}
}
