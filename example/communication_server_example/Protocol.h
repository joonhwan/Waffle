#pragma once

// sample protocol header
// containing 4 parts
// see below example

#include "packet/WPacket.h"
#include <QDataStream>
#include <QDateTime>
#include <QMetaType>
#include <QString>

// 1) port number
#define TRIP_SERVER_PORT 15600

// 2) protocol message id
struct TripPlanProtocol
{
	enum {
		COMMAND = 1,
		QUERY = 2,
		RESULT = 3,
	};
};

// 3) protocol data strcture ( XxxxData type )
//  NOTE: MUST HAVE
//    - void serialize(WPacketStream&, quint32);
//    - VERSION enum
//    - PACKETID enum
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
// 4) protocol data packet placeholder (for easy usage) (XxxxPacket type)
typedef WPacketT<TripPlanCommandData> TripPlanCommandPacket;
Q_DECLARE_METATYPE(TripPlanCommandPacket);

//  3), 4), 3), 4), 3) ,4) ....
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

