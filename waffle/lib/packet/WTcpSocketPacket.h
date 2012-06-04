#pragma once

#include "WTcpSocket.h"
#include "WTcpSocketStream.h"

class WTcpSocketPacket
{
public:
	virtual int id() const = 0;
	virtual QByteArray toBlock() = 0;
	virtual bool fromBlock(const QByteArray& block) = 0;
protected:
	friend class WTcpSocket; // for accessing 'readFrom' protectively
};

// type 'T' is Data type holding all packet contents.
// following spec is needed
//  - enum T::VERSION contants
//  - int id; packet id
//  - void T::serialize(WTcpSocketStream& s, quint32 version);
template<typename T>
class WTcpSocketPacketT : public T
						, public WTcpSocketPacket
{
public:
	virtual int id() const {
		return T::PACKETID;
	}
	virtual QByteArray toBlock() {
		QByteArray byteArray;
		WTcpSocketStream stream(byteArray, false);
		quint32 version = (quint32)T::VERSION;
		stream << version;
		T::serialize(stream, version);
		return byteArray;
	}
	virtual bool fromBlock(const QByteArray& block) {
		WTcpSocketStream stream(block);
		quint32 version = 0;
		stream >> version;
		T::serialize(stream, version);
		return stream.status()==QDataStream::Ok;
	}
protected:
};
