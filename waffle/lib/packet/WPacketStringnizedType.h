#pragma once

#include <stdio.h>

template<typename T, int C, char F>
class WPacketStringizedType
{
public:
	enum {
		VALUE_LEN = C,
		BUFFER_LEN = 256,
	};
    WPacketStringizedType(const T& value = 0)
		: m_value(value)
	{
	}
    virtual ~WPacketStringizedType()
	{
	}
	WPacketStringizedType<T,C,F>& operator=(WPacketStringizedType<T,C,F>& other)
	{
		m_value = other.m_value;
		return *this;
	}
	WPacketStringizedType<T,C,F>& operator=(const T& value)
	{
		m_value = value;
		return *this;
	}
	bool operator==(const WPacketStringizedType<T,C,F>& other) const
	{
		return m_value == other.m_value;
	}
	bool operator!=(const WPacketStringizedType<T,C,F>& other) const
	{
		return m_value != other.m_value;
	}
	bool operator==(const T& value) const
	{
		return m_value==value;
	}
	bool operator!=(const T& value) const
	{
		return m_value!=value;
	}
	const char* format() const
	{
		return s_format.constData();
	}
	T value() const
	{
		return m_value;
	}
	static QByteArray createFormat()
	{
		return
			(QString::fromLatin1("%0") +
			 QString::fromLatin1("%1%2")
			 .arg(C)
			 .arg(F)
				).toAscii();
	}
	void writeToStream(QDataStream& s) const
	{
		char valString[BUFFER_LEN];
		::sprintf_s(valString, BUFFER_LEN, format(), m_value);
		s.writeRawData(valString, VALUE_LEN); // copy only 'C' characters.
	}
	void readFromStream(QDataStream& s)
	{
		char valString[BUFFER_LEN];
		s.readRawData(valString, VALUE_LEN);
		int r = ::_snscanf_s(valString, VALUE_LEN, format(), &m_value);
		Q_ASSERT(r==1);
	}
	QByteArray toAsciiString()
	{
		QByteArray v(VALUE_LEN, 0);
		char valString[BUFFER_LEN];
		::sprintf_s(valString, BUFFER_LEN, format(), m_value);
		::memcpy(v.data(), valString, VALUE_LEN);
		return v;
	}
	void fromAsciiString(QByteArray& array)
	{
		int r = ::_snscanf_s(array.data(), array.length(), format(), &m_value);
		Q_ASSERT(r==1);
	}
	int fixedLength() const {
		return VALUE_LEN;
	}
protected:
	T m_value;
	static QByteArray s_format;
};

template<typename T, int C, char F>
QByteArray WPacketStringizedType<T,C,F>::s_format = WPacketStringizedType<T,C,F>::createFormat();

template<typename T, int C, char F>
QDataStream& operator << (QDataStream& s, const WPacketStringizedType<T,C,F>& obj)
{
	obj.writeToStream(s);
	return s;
}

template<typename T, int C, char F>
QDataStream& operator >> (QDataStream& s, WPacketStringizedType<T,C,F>& obj)
{
	obj.readFromStream(s);
	return s;
}


