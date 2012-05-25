#include "WSqlTableFieldNameList.h"

WSqlTableFieldNameList::WSqlTableFieldNameList(const QString& tableName, QSqlDatabase db)
{
	QSqlQuery query(QString("SELECT * FROM %1").arg(tableName), db);
	buildListFrom(query);
}

WSqlTableFieldNameList::WSqlTableFieldNameList(QSqlQuery& query)
{
	buildListFrom(query);
}

//virtual
WSqlTableFieldNameList::~WSqlTableFieldNameList()
{
}

bool WSqlTableFieldNameList::buildListFrom(QSqlQuery& query)
{
	bool built = false;
	this->clear();
	if (query.isActive()) {
		QSqlRecord fields = query.record();
		for (int i = 0; i < fields.count(); ++i) {
			this->append(fields.fieldName(i));
		}
		built = true;
	}
	return built;
}


std::ostream& operator << (std::ostream& os, const WSqlTableFieldNameList& me)
{
	int index = 0;
	foreach(const QString& field, me) {
		if (index) {
			os << ", ";
		}
		++index;
		os << field.toLocal8Bit().data();
	}
	return os;
}
