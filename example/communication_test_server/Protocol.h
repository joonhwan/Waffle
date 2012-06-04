#pragma once

#include "packet/WPacket.h"
#include <QDataStream>
#include <QDateTime>
#include <QMetaType>
#include <QString>

#define TRIP_SERVER_PORT 15600

struct TripPlanProtocol
{
	enum {
		COMMAND = 1,
		QUERY = 2,
		RESULT = 3,
	};
};

struct TripPlanCommandData
{
	QString command;

	enum {
		VERSION = 1,
		PACKETID = TripPlanProtocol::COMMAND,
	};
	void serialize(WPacketStream& stream, quint32 version) {
		stream & command;
	}
};
typedef WPacketT<TripPlanCommandData> TripPlanCommandPacket;
Q_DECLARE_METATYPE(TripPlanCommandPacket);

struct TripPlanQueryData
{
	QString from;
	QString to;
	QDateTime time;
	bool isDeparture;

	enum  {
		VERSION = 1,
		PACKETID = TripPlanProtocol::QUERY,
	};
	void serialize(WPacketStream& stream, quint32 version) {
		stream & from;
		stream & to;
		stream & time;
		stream & isDeparture;
	}
};
typedef WPacketT<TripPlanQueryData> TripPlanQueryPacket;
Q_DECLARE_METATYPE(TripPlanQueryPacket);

struct TripPlanResultData
{
	QDateTime departure;
	quint16 duration;
	quint8 changes;
	QString trainType;

	enum  {
		VERSION = 1,
		PACKETID = TripPlanProtocol::RESULT,
	};
	void serialize(WPacketStream& stream, quint32 version) {
		stream & departure;
		stream & duration;
		stream & changes;
		stream & trainType;
	}
};
typedef WPacketT<TripPlanResultData> TripPlanResultPacket;
Q_DECLARE_METATYPE(TripPlanResultPacket);

inline
void initializeProtocol()
{
	qRegisterMetaType<TripPlanResultPacket>();
	qRegisterMetaType<TripPlanQueryPacket>();
}

