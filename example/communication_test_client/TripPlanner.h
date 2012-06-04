#pragma once

#include "Protocol.h"
#include <QObject>

class WTcpSocket;

class TripPlanner : public QObject
{
    Q_OBJECT
public:
    TripPlanner(QObject* parent=0);
    virtual ~TripPlanner();
	void setQuery(TripPlanQueryPacket& query);
	void search(TripPlanQueryPacket& query);
signals:
	void received(const TripPlanResultPacket& resultItem);
	void stateChanged(const QString& description);
	void stoppedSearch();
	void error();
public slots:
	void search();
	void stopSearch();
private slots:
	void sendRequest();
	void onDisconnectedByServer();
	void onReceive(quint32 packetId, const QByteArray& block);
private:
	WTcpSocket* m_socket;
	TripPlanQueryPacket m_query;
	int m_nextBlockSize;
};
