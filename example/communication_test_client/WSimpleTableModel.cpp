#include "WSimpleTableModel.h"

WSimpleTableModel::WSimpleTableModel(Qt::Orientation orientation, QObject* parent)
	: QAbstractTableModel(parent)
	, m_orientation(orientation)
	, m_dataIndexHeaderFormat("%1")
{
}

//virtual
WSimpleTableModel::~WSimpleTableModel()
{
}

//virtual
int WSimpleTableModel::rowCount(const QModelIndex& parent) const
{
	if (m_orientation==Qt::Vertical) {
		return dataCount();
	} else {
		return fieldCount();
	}
}

//virtual
int WSimpleTableModel::columnCount(const QModelIndex& parent) const
{
	if (m_orientation==Qt::Vertical) {
		return fieldCount();
	} else {
		return dataCount();
	}
}

//virtual
QVariant WSimpleTableModel::data(const QModelIndex& index, int role) const
{
	QVariant v;
	if (validIndex(index) && role==Qt::DisplayRole) {
		v = tableData(dataIndexOf(index), fieldIndexOf(index), Cell);
	}
	return v;
}

//virtual
QVariant WSimpleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant v;
	if (role == Qt::DisplayRole) {
		if (orientation != m_orientation) {
			if ( (section < fieldCount()) ) {
				v = tableData(0, section, Header);
			}
		} else {
			v = QString(m_dataIndexHeaderFormat).arg(section);
		}
	}
	return v;
}
