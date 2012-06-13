#pragma once

#include "packet/WPacket.h"
#include "packet/WPacketStream.h"
#include "packet/WPacketFixedLenString.h"
#include "packet/WPacketStringnizedType.h"
#include "packet/WPacketHeadTailModel.h"
#include <memory.h>
#include <stdio.h>

#define PACKETTEST_SERVER_PORT

struct PacketTestProtocol
{
	enum {
		SIMPLE = 1,
	};
};


struct SimpleData
{
	QString message;

	enum {
		VERSION = 1,
		PACKETID = PacketTestProtocol::SIMPLE
	};
	void serialize(WPacketStream& stream, quint32 version) {
		stream & message;
	}
};

struct AvatecLedCommandData
{
	WPacketFixedLenString<4> command;
	void serialize(WPacketStream& stream, quint32 version) {
		stream & command;
	}
};
typedef WPacketSimpleT<AvatecLedCommandData> AvatecLedCommandPacket;

struct AvatecLedSetBrightnessData : AvatecLedCommandData
{
	WPacketStringizedType<int, 3, 'd'> brightness;

	void serialize(WPacketStream& stream, quint32 version) {
		AvatecLedCommandData::serialize(stream, version);
		stream & brightness;
	}
};

typedef WPacketSimpleT<AvatecLedSetBrightnessData> AvatecLedSetBrightnessPacket;

class DummyHardwarePacketModel : public WPacketHeadTailModel
{
public:
	DummyHardwarePacketModel(QObject* parent = 0)
		: WPacketHeadTailModel(QByteArray(1,'\x02'),
							   QByteArray(1,'\x0d'),
							   parent)
	{
	}
	virtual QByteArray beginWrite(WPacket& packet)
	{
		// body = body + checksum!
		QByteArray body = packet.toBlock();
		short checkSum = checksum(body);

		QByteArray ba;
		ba.append(m_head);
		// body = body + checksum
		ba.append(body);
		ba.append((const char*)&checksum, 2);
		ba.append(m_tail);
		return ba;
	}
	static short checksum(const QByteArray& response)
	{
		// from dumb LED controller used in 'A' site.
		int sum = 0;
		sum += 0x02;
		sum += 0x0d;
		for(int i=0; i<response.length(); ++i)
		{
			char val = (char)(response[i]);
			sum += val;
		}
		sum = (~sum)+1;

		char tmpBuffer[32];
		sprintf_s(tmpBuffer, 32, "%08X", sum);
		short checkSum = 0;
		char* p = (char*)&checkSum;
		*p++ = tmpBuffer[6];
		*p = tmpBuffer[7];

		return checkSum;
	}
};
