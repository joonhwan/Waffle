#pragma once

#include "Protocol.h"
#include "WSimpleTableModelT.h"
#include <QAbstractItemModel>
#include <QList>

class TripPlanResultModel : public WSimpleTableModelT<TripPlanResultPacket>
{
public:
	TripPlanResultModel(QObject* parent);
protected:
	virtual QString tableData(int dataIndex, int fieldIndex, TableDataRole role) const;
	virtual int fieldCount() const;
};

