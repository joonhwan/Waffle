#pragma once

#include "packet/WPacketIo.h"

// T is communication class derived from QIODevice
template<typename T>
class WPacketIoT : public WPacketIo
{
public:
    WPacketIoT(WPacketModel* packetModel,
			   QObject* parent)
		: WPacketIo(packetModel,
					new T(parent),
					parent)
	{
	}
    virtual ~WPacketIoT()
	{
	}
	T* io() {
		T* p = qobject_cast<T*>(WPacketIo::io());
		Q_ASSERT(p);
		return p;
	}
	const T* io() const {
		const T* p = qobject_cast<const T*>(WPacketIo::io());
		Q_ASSERT(p);
		return p;
	}
};
