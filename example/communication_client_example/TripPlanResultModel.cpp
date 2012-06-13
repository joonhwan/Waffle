#include "TripPlanResultModel.h"
#include <QStringList>

TripPlanResultModel::TripPlanResultModel(QObject* parent)
	: WSimpleTableModelT<TripPlanResultPacket>(Qt::Vertical, parent)
{
}

//virtual
QString TripPlanResultModel::tableData(int dataIndex, int fieldIndex, TableDataRole role) const
{
	const TripPlanResultPacket* d = 0;
	if (role == Cell) {
		d = &item(dataIndex);
	}
	switch (fieldIndex) {
	case 0:
		switch (role) {
		case Cell:
			return d->departure.date().toString(Qt::LocalDate);
		case Header:
			return "Date";
		}
		break;
	case 1:
		switch (role) {
		case Cell:
			return d->departure.time().toString(tr("hh:mm"));
		case Header:
			return "Departure";
		}
		break;
	case 2:
		switch (role) {
		case Cell:
			return d->departure.time().addSecs(d->duration).toString(tr("hh:mm"));
		case Header:
			return "Arrival";
		}
		break;
	case 3:
		switch (role) {
		case Cell:
			return tr("%1 hr %2 min").arg(d->duration / 60).arg(d->duration % 60);
		case Header:
			return "Arrival";
		}
		break;
	case 4:
		switch (role) {
		case Cell:
			return QString::number(d->changes);
		case Header:
			return "Changes";
		}
		break;
	case 5:
		switch (role) {
		case Cell:
			return d->trainType;;
		case Header:
			return "TrainType";
		}
		break;
	}
	return QString();
}

//virtual
int TripPlanResultModel::fieldCount() const
{
	return 6;
}
