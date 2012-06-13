#pragma once

#include <QDataStream>
#include <stdlib.h>

template<int L>
class WPacketFixedLenString
{
public:
	enum {
		BUFFER_LEN = L
	};
	WPacketFixedLenString(const char* v = 0)
	{
		if (v) {
			Q_ASSERT(strlen(v)==BUFFER_LEN);
			::memcpy(m_buffer, v, BUFFER_LEN);
		} else {
			::memset(m_buffer, 0, BUFFER_LEN);
		}
	}
	bool operator==(const WPacketFixedLenString<L>& other) const
	{
		return 0==::memcmp(m_buffer, other.m_buffer, BUFFER_LEN);
	}
	bool operator!=(const WPacketFixedLenString<L>& other) const
	{
		return !((*this)==other);
	}
	WPacketFixedLenString& operator=(const WPacketFixedLenString<L>& other)
	{
		::memcpy(m_buffer, other.m_buffer, BUFFER_LEN);
		return *this;
	}
	void writeToStream(QDataStream& s) const
	{
		s.writeRawData(m_buffer, BUFFER_LEN);
	}
	void readFromStream(QDataStream& s)
	{
		s.readRawData(m_buffer, BUFFER_LEN);
	}
	int fixedLength() const {
		return BUFFER_LEN;
	}
protected:
	char m_buffer[BUFFER_LEN];
};

template<int L>
QDataStream& operator << (QDataStream& s, const WPacketFixedLenString<L>& obj)
{
	obj.writeToStream(s);
	return s;
}

template<int L>
QDataStream& operator >> (QDataStream& s, WPacketFixedLenString<L>& obj)
{
	obj.readFromStream(s);
	return s;
}
