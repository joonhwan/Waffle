#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <QString>

#define DEFINE_OP(TYPE)								\
	WPacketStream& operator & (TYPE& value) {	\
		if (m_isLoading) {							\
			(*this) >> value;						\
		} else {									\
			(*this) << value;						\
		}											\
		return *this;								\
	}

namespace Wf {

enum StreamDirection {
	DataToStream = 0,
	StreamToData,
};

};

class WPacketStream : public QDataStream
{
public:
	WPacketStream(QByteArray& byteArray, bool loading);
	WPacketStream(QByteArray& byteArray, Wf::StreamDirection direction);
	WPacketStream(const QByteArray& byteArray);
	WPacketStream(QIODevice& io, bool loading);
	virtual ~WPacketStream();
	bool isLoading() const;
	bool isStreamToData() const;

	// '&' operators for easy and maintainable serialization
	// DEFINE_OP(QDateTime)
	// DEFINE_OP(QString)
	// DEFINE_OP(bool)
	// DEFINE_OP(qreal)
	// DEFINE_OP(quint32)
	// DEFINE_OP(quint16)
	// DEFINE_OP(quint8)
	// DEFINE_OP(qint32)
	// DEFINE_OP(qint16)
	// DEFINE_OP(qint8)
	// DEFINE_OP(QByteArray)
	template<typename T>
	WPacketStream& operator& (T& value) {
		if (m_isLoading) {
			(*this) >> value;
		} else {
			(*this) << value;
		}
		return *this;
	}

	// template<typename T, int C>
	// WPacketStream& operator& (WPacketStringizedIntType<T,C>& value) {
	// 	char valStr[C] = { '\0', };
	// 	if (m_isLoading) {
	// 		sprintf_s(valStr, C, 

	// more to come..
	;

protected:
	bool m_isLoading;
};

inline
bool WPacketStream::isLoading() const
{
	return m_isLoading;
}

inline
bool WPacketStream::isStreamToData() const
{
	return m_isLoading;
}
