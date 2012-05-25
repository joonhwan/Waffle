#include "WSqlQueryBuilder.h"
#include "common/global/WGlobal.h"
#include <QTextStream>

WSqlSimpleInsertQueryBuilder::WSqlSimpleInsertQueryBuilder(const QString& tableName,
											   QSqlDatabase db)
	: m_tableName(tableName)
	, m_db(db)
	, m_useQuote(true)
{
}

//virtual
WSqlSimpleInsertQueryBuilder::~WSqlSimpleInsertQueryBuilder()
{
}

WSqlSimpleInsertQueryBuilder&
WSqlSimpleInsertQueryBuilder::useQuote(bool use)
{
	m_useQuote = use;
	return *this;
}

WSqlSimpleInsertQueryBuilder&
WSqlSimpleInsertQueryBuilder::keyValue(const QString& fieldName, const QVariant& value)
{
	m_fields[fieldName] = value;
	return *this;
}

QSqlQuery
WSqlSimpleInsertQueryBuilder::query() const
{
	QSqlDriver* driver = m_db.driver();

	// QString form;
	// QTextStream os(&form, QIODevice::WriteOnly);

	// os << "INSERT INTO " << m_tableName << " (";
	// int index = 0;
	// QVariantMap::const_iterator it = m_fields.constBegin();
	// for (; it != m_fields.constEnd(); ++it) {
	// 	if (index) {
	// 		os << QChar(',');
	// 	}
	// 	os << (m_useQuote ? wQuoted(it.key()) : it.key());
	// }

	// os << ") VALUES (";

	// index = 0;
	// it = m_fields.constBegin();
	// for (; it != m_fields.constEnd(); ++it) {
	// 	if (index) {
	// 		os << QChar(',');
	// 	}
	// 	// let each driver code convert value into string representation correctly.
	// 	QSqlField dummy(it.key(), it.value().type());
	// 	dummy.setValue(it.value());
	// 	os << driver->formatValue(dummy);

	// 	++index;
	// }

	// os << ")";
	// os << flush;

	QString field;
	QString value;

	int index = 0;
	QVariantMap::const_iterator it = m_fields.constBegin();
	for (; it != m_fields.constEnd(); ++it) {
		if (index) {
			field += QChar(',');
			value += QChar(',');
		}

		QSqlField tmpField(it.key(), it.value().type());
		tmpField.setValue(it.value());

		field += (m_useQuote ? wQuoted(it.key()) : it.key());
		value += driver->formatValue(tmpField);

		++index;
	}

	QString form =
		QString("INSERT INTO %1 (%2) VALUES (%3)")
		.arg(m_tableName)
		.arg(field)
		.arg(value);

	return QSqlQuery(form);
}
