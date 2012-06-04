#pragma once

#include "WSimpleTableModel.h"
#include <QList>
#include <QVariant>

template<typename T>
class WSimpleTableModelT : public WSimpleTableModel
{
public:
	WSimpleTableModelT(Qt::Orientation orientation=Qt::Vertical, QObject* parent=0)
		: WSimpleTableModel(orientation, parent)
	{
	}
	void addData(const T& data) {
		int indexToAdd = m_modelDataList.size();
		if (vertical()) {
			beginInsertRows(QModelIndex(), indexToAdd, indexToAdd);
		} else {
			beginInsertColumns(QModelIndex(), indexToAdd, indexToAdd);
		}
		m_modelDataList.push_back(data);
		if (vertical()) {
			endInsertRows();
		} else {
			endInsertColumns();
		}
	}
	void clear() {
		int count = dataCount();
		if (count) {
			if (vertical()) {
				beginRemoveRows(QModelIndex(), 0, count-1);
			} else {
				beginRemoveColumns(QModelIndex(), 0, count-1);
			}
			m_modelDataList.clear();
			if (vertical()) {
				endRemoveRows();
			} else {
				endRemoveColumns();
			}
		}
	}
protected:
	// virtual QString tableData(int dataIndex, int fieldIndex, TableDataRole role) const  = 0
	// virtual int fieldCount() const = 0;
	virtual int dataCount() const {
		return m_modelDataList.size();
	}
	const T& item(int dataIndex) const {
		return m_modelDataList[dataIndex];
	}
	T& item(int dataIndex) {
		return m_modelDataList[dataIndex];
	}
	QList<T> m_modelDataList;
};

