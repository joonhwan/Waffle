#include "WSqlQueryBinder.h"

WSqlQueryBinder::WSqlQueryBinder(QSqlQuery& query)
	: m_query(query)
{
}

//virtual
WSqlQueryBinder::~WSqlQueryBinder()
{
	m_query.exec();
}

WSqlQueryBinder& WSqlQueryBinder::bind(const QString& field,
									   const QVariant& value)
{
	m_query.bindValue(QString(":%1").arg(field), value);
	return *this;
}
