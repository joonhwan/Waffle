#pragma once

#include <QAbstractTableModel>

class WSimpleTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
	enum TableDataRole {
		Cell,
		Header,
	};
    WSimpleTableModel(Qt::Orientation orientation=Qt::Vertical, QObject* parent=0);
    virtual ~WSimpleTableModel();
	virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool horizontal() const;
	bool vertical() const;
protected:
	virtual QString tableData(int dataIndex, int fieldIndex, TableDataRole role) const = 0;
	virtual int dataCount() const = 0;
	virtual int fieldCount() const = 0;
	bool validIndex(const QModelIndex& index) const;
	int dataIndexOf(const QModelIndex& index) const;
	int fieldIndexOf(const QModelIndex& index) const;
private:
	Qt::Orientation m_orientation;
	QString m_dataIndexHeaderFormat;
};

inline
bool WSimpleTableModel::horizontal() const
{
	return m_orientation == Qt::Horizontal;
}

inline
bool WSimpleTableModel::vertical() const
{
	return !horizontal();
}

inline
bool WSimpleTableModel::validIndex(const QModelIndex& index) const
{
	int row = index.row();
	int col = index.column();
	return index.isValid()
		&& ((vertical() && row < dataCount() && col < fieldCount())
			|| (horizontal() && row < fieldCount() && col < dataCount()));
}

inline
int WSimpleTableModel::dataIndexOf(const QModelIndex& index) const
{
	if (vertical()) {
		return index.row();
	} else {
		return index.column();
	}
}

inline
int WSimpleTableModel::fieldIndexOf(const QModelIndex& index) const
{
	if (vertical()) {
		return index.column();
	} else {
		return index.row();
	}
}
