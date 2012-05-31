#include "WSqlRelationalTableModel.h"

WSqlRelationalTableModel(QObject* parent, QSqlDatabase db)
	: QSqlRelationalTableModel(parent, db)
{
}

//virtual
WSqlRelationalTableModel::~WSqlRelationalTableModel()
{
}

//virtual
bool WSqlRelationalTableModel::setTable(const QString& table)
{
	bool done = false;

	do
	{
		QSqlTableModel m;
		m.setTable(table);
		if (!m.select()) {
			break;
		}
		m_fieldInfo = m.record();
		if (m_fieldInfo.isEmpty()) {
			break;
		}

		QSqlRelationalTableModel::setTable(table);

		done = true;
	} while (0);

	return done;
}

