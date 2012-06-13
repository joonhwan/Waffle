#include "WPacketStream.h"

WPacketStream::WPacketStream(QByteArray& byteArray, bool loading)
	: QDataStream(&byteArray, loading?QIODevice::ReadOnly:QIODevice::WriteOnly)
	, m_isLoading(loading)
{
}

WPacketStream::WPacketStream(QByteArray& byteArray, Wf::StreamDirection direction)
	: QDataStream(&byteArray, direction==Wf::StreamToData?QIODevice::ReadOnly:QIODevice::WriteOnly)
	, m_isLoading(direction==Wf::StreamToData)
{
}


WPacketStream::WPacketStream(const QByteArray& byteArray)
	: QDataStream(byteArray)
	, m_isLoading(true)
{
}

WPacketStream::WPacketStream(QIODevice& io, bool loading)
	: QDataStream(&io)
	, m_isLoading(loading)
{
}

//virtual
WPacketStream::~WPacketStream()
{
}
