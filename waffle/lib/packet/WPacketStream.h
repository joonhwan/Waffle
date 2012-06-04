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

class WPacketStream : public QDataStream
{
public:
	WPacketStream(QByteArray& byteArray, bool loading);
	WPacketStream(const QByteArray& byteArray);
	WPacketStream(QIODevice& io, bool loading);
	virtual ~WPacketStream();
	bool isLoading();

	// '&' operators for easy and maintainable serialization
	DEFINE_OP(QDateTime)
	DEFINE_OP(QString)
	DEFINE_OP(bool)
	DEFINE_OP(qreal)
	DEFINE_OP(quint32)
	DEFINE_OP(quint16)
	DEFINE_OP(quint8)
	DEFINE_OP(qint32)
	DEFINE_OP(qint16)
	DEFINE_OP(qint8)
	// more to come..
	;
protected:
	bool m_isLoading;
};

inline
bool WPacketStream::isLoading()
{
	return m_isLoading;
}
