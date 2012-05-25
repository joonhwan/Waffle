#pragma once

#include <QtGlobal>

class WRandom
{
public:
	WRandom(qint32 min, qint32 max);
	WRandom(quint32 range);
	int generate() const;
protected:
	qint32 m_min;
	quint32 m_range;
};

inline
int WRandom::generate() const
{
	return m_min + qrand() % m_range;
}
