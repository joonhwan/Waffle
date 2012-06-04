#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <QString>

#define DEFINE_OP(TYPE)								\
	WTcpSocketStream& operator & (TYPE& value) {	\
		if (m_isLoading) {							\
			(*this) >> value;						\
		} else {									\
			(*this) << value;						\
		}											\
		return *this;								\
	}

class WTcpSocketStream : public QDataStream
{
public:
	WTcpSocketStream(QByteArray& byteArray, bool loading);
	WTcpSocketStream(const QByteArray& byteArray);
	WTcpSocketStream(QIODevice& io, bool loading);
	virtual ~WTcpSocketStream();
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
bool WTcpSocketStream::isLoading()
{
	return m_isLoading;
}
