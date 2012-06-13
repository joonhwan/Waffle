#pragma once

#include "WPacketIo.h"
#include "WPacketStream.h"

class WPacket
{
public:
	virtual int id() const = 0;
	virtual QByteArray toBlock() = 0;
	virtual bool fromBlock(QByteArray& block) = 0;
protected:
	friend class WPacketIo; // for accessing 'readFrom' protectively
};

// a general packet class template for 3rdparty sw/hw
template<typename T>
class WPacketSimpleT : public T
					 , public WPacket
{
public:
	virtual int id() const {
		// meaningless when using WPacketSimpleT.
		return 0;
	}
	virtual QByteArray toBlock() {
		QByteArray byteArray;
		WPacketStream stream(byteArray, Wf::DataToStream);
		T::serialize(stream, 0);
		return byteArray;
	}
	virtual bool fromBlock(QByteArray& block) {
		WPacketStream stream(block, Wf::StreamToData);
		T::serialize(stream, 0);
		return stream.status()==QDataStream::Ok;
	}
protected:
};

// type 'T' is Data type holding all packet contents.
// following spec is needed
//  - enum T::VERSION contants
//  - int id; packet id
//  - void T::serialize(WPacketStream& s, quint32 version);
template<typename T>
class WPacketT : public T
			   , public WPacket
{
public:
	virtual int id() const {
		return T::PACKETID;
	}
	virtual QByteArray toBlock() {
		QByteArray byteArray;
		WPacketStream stream(byteArray, Wf::DataToStream);
		quint32 version = (quint32)T::VERSION;
		stream << version;
		T::serialize(stream, version);
		return byteArray;
	}
	virtual bool fromBlock(QByteArray& block) {
		WPacketStream stream(block, Wf::StreamToData);
		quint32 version = 0;
		stream >> version;
		T::serialize(stream, version);
		return stream.status()==QDataStream::Ok;
	}
protected:
};

