#include "WRandom.h"
#include <QDateTime>

namespace {

struct Randomizer {
	Randomizer()
	{
		qsrand(QDateTime::currentDateTime().toTime_t());
	}
} radomizer;

}

WRandom::WRandom(qint32 min, qint32 max)
{
	m_min = qMin(min, max);
	m_range = (quint32)(qAbs(max - min) + 1);
}

WRandom::WRandom(quint32 range)
{
	m_min = 0;
	m_range = range;
}

