#include "WTcpSocketStream.h"

WTcpSocketStream::WTcpSocketStream(QByteArray& byteArray, bool loading)
	: QDataStream(&byteArray, loading?QIODevice::ReadOnly:QIODevice::WriteOnly)
	, m_isLoading(loading)
{
}

WTcpSocketStream::WTcpSocketStream(const QByteArray& byteArray)
	: QDataStream(byteArray)
	, m_isLoading(true)
{
}

WTcpSocketStream::WTcpSocketStream(QIODevice& io, bool loading)
	: QDataStream(&io)
	, m_isLoading(loading)
{
}

//virtual
WTcpSocketStream::~WTcpSocketStream()
{
}
