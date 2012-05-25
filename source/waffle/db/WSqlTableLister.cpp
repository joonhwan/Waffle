#include "WSqlTableLister.h"
#include <QVariant>

WSqlTableLister::WSqlTableLister(const QString& tableName,
								 int maxRecordToList,
								 QSqlDatabase db)
	: m_tableName(tableName)
	, m_maxRecordToList(maxRecordToList)
	, m_db(db)
{
}

QSqlQuery WSqlTableLister::query() const
{
	return QSqlQuery(QString("SELECT * from %1")
					 .arg(m_tableName));
}


std::ostream& operator << (std::ostream& os, WSqlTableLister& me)
{
	QSqlQuery query = me.query();
	if (query.isActive()) {
		int row = 0;
		int maxList = me.maxList();
		while (query.next()) {
			QSqlRecord record = query.record();
			for (int i = 0; i < record.count(); ++i) {
				if (i) {
					os << ", ";
				}
				os << record.value(i).toString().toLocal8Bit().data();
			}
			os << std::endl;

			if (maxList && row >= maxList) {
				os << "...." << std::endl;
				break;
			}
			++row;
		}
	} else {
		os << "failed in table listing : "
		   << query.lastError().text().toLocal8Bit().data()
		   << std::endl;
	}
	return os;
}
