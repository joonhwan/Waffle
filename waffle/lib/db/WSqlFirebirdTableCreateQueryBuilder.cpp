#include "WSqlFirebirdTableCreateQueryBuilder.h"
#include "common/global/WException.h"
#include <QUuid>
#include <QDebug>

namespace {

const QChar LF('\n');
const int UUID_STRING_LEN = QUuid().toByteArray().size();

};

WSqlFirebirdTableCreateQueryBuilder::WSqlFirebirdTableCreateQueryBuilder(const QString& tableName,
																		 QSqlDatabase db)
	: m_db(db)
	, m_tableName(tableName)
	, m_useQuotedFieldName(true)
{
}

//virtual
WSqlFirebirdTableCreateQueryBuilder::~WSqlFirebirdTableCreateQueryBuilder()
{
}

void WSqlFirebirdTableCreateQueryBuilder::setUseQuotedFieldName(bool quote)
{
	m_useQuotedFieldName = quote;
}

// calling order decide field's position
WSqlFirebirdTableCreateQueryBuilder&
WSqlFirebirdTableCreateQueryBuilder::field(const QString& fieldName,
										   Wf::DbType dbType, Wf::DbFieldAttribute attribute)
{
	QString dbTypeString;
	switch (dbType) {
	case Wf::DbSmallInt:
	case Wf::DbInt:
	case Wf::DbBigInt:
	case Wf::DbFloat:
	case Wf::DbDouble:
	case Wf::DbDate:
		break;
	default:
		// handle error here?
		throw WException(QObject::tr("Invalid DB Field Defintion Error."));
		break;
	}

	FieldDefinition fd;
	fd.fieldName = fieldName;
	fd.dbType = dbType;
	fd.attribute = attribute;
	fd.length = 0;

	return field(fd);
}

WSqlFirebirdTableCreateQueryBuilder&
WSqlFirebirdTableCreateQueryBuilder::field(const QString& fieldName,
										   Wf::DbType dbType, int length, Wf::DbFieldAttribute attribute)
{
	QString dbTypeString;
	switch (dbType) {
	case Wf::DbSmallInt:
	case Wf::DbInt:
	case Wf::DbBigInt:
	case Wf::DbFloat:
	case Wf::DbDouble:
		// handle error here?
		throw WException(QObject::tr("Invalid DB Field Defintion Error."));
		break;
	default:
		break;
	}

	FieldDefinition fd;
	fd.fieldName = fieldName;
	fd.dbType = dbType;
	fd.attribute = attribute;
	fd.length = length;

	return field(fd);
}

WSqlFirebirdTableCreateQueryBuilder&
WSqlFirebirdTableCreateQueryBuilder::field(FieldDefinition& fieldDefinition)
{
	m_fieldDefinitionList.push_back(fieldDefinition);
	return *this;
}

WSqlFirebirdTableCreateQueryBuilder&
WSqlFirebirdTableCreateQueryBuilder::serialPrimaryKey(const QString& fieldName)
{
	int baseNumber = 1;

	// QString statement = QString("%1 BIGINT NOT NULL")
	// 					.arg(normalize(fieldName));
	// m_ddl << statement;
	FieldDefinition fd;
	fd.fieldName = fieldName;
	fd.dbType = Wf::DbBigInt;
	fd.attribute = Wf::DbNotNull;
	fd.length = 0;
	field(fd);

	// PK_ --> PrimaryKey_
	QString constraintName = QString("PK_%1").arg(m_tableName).toUpper();
	QString constraint = (QStringList()
						  << "CONSTRAINT %1"
						  << "  PRIMARY KEY(%2)").join(LF)
						 .arg(normalize(constraintName))
						 .arg(normalize(fieldName));
	m_constraints << constraint;

	// GEN_PK_ --> GENerator_PrimaryKey_
	QString generatorName = QString("GEN_PK_%1_OF_%2")
							.arg(fieldName)
							.arg(m_tableName).toUpper();
	// _BI --> _Before_Insert
	QString triggerName = QString("%1_BI").arg(m_tableName).toUpper();
	QString psql;
	psql = (QStringList()
			<< "EXECUTE BLOCK AS BEGIN"
			<< "  IF (NOT EXISTS(SELECT 1 "
			<< "                 FROM RDB$GENERATORS "
			<< "                 where RDB$GENERATOR_NAME = '%1')) THEN"
			<< "    EXECUTE STATEMENT 'CREATE GENERATOR \"%1\"';"
			<< "END").join(LF)
		   .arg(generatorName);
	m_psql << psql;

	psql = (QStringList()
			<< "CREATE OR ALTER TRIGGER \"%2\" FOR %3"
			<< "ACTIVE BEFORE INSERT AS BEGIN"
			<< "  IF (NEW.%4 IS NULL) THEN"
			<< "    NEW.%4 = GEN_ID(\"%1\", %5);"
			<< "END").join(LF)
		   .arg(generatorName)
		   .arg(triggerName)
		   .arg(m_tableName)
		   .arg(normalize(fieldName))
		   .arg(baseNumber);
	m_psql << psql;

	return *this;
}

// WSqlFirebirdTableCreateQueryBuilder& WSqlFirebirdTableCreateQueryBuilder::uuidPrimaryKey(const QString& fieldName)
// {
// 	QString statement = QString("%1 VARCHAR(%2) PRIMARY KEY")
// 						.arg(normalize(fieldName))
// 						.arg(UUID_STRING_LEN);
// 	m_ddl << statement;
// 	return *this;
// }

WSqlFirebirdTableCreateQueryBuilder& WSqlFirebirdTableCreateQueryBuilder::foreignSerialKey(const QString& fieldName,
																						   const QString& foreignTable,
																						   const QString& foreignTableKey)
{
	// QString keyType = "BIGINT";
	// QString statement = QString("%1 %2 NOT NULL")
	// 					.arg(normalize(fieldName))
	// 					.arg(keyType);
	// m_ddl << statement;
	FieldDefinition fd;
	fd.fieldName = fieldName;
	fd.dbType = Wf::DbBigInt;
	fd.attribute = Wf::DbNotNull;
	fd.length = 0;
	field(fd);

	QString constraint = (QStringList()
						  << "CONSTRAINT FK_%1"
						  << "  FOREIGN KEY (%2)"
						  << "  REFERENCES %3(%4)"
						  << "  ON DELETE CASCADE"
						  << "  ON UPDATE CASCADE").join(LF)
						 .arg(fieldName)
						 .arg(normalize(fieldName))
						 .arg(foreignTable)
						 .arg(normalize(foreignTableKey));
	m_constraints << constraint;

	return *this;
}

QSqlQuery WSqlFirebirdTableCreateQueryBuilder::query() const
{
	QString form = QString::fromLatin1("CREATE TABLE %1 (\n").arg(m_tableName);
	int index = 0;

	QSqlQuery query;

	// foreach(const QString& s, m_ddl) {
	// 	if (index) {
	// 		form += ",\n";
	// 	}
	// 	++index;
	// 	form += s;
	// }
	foreach(const FieldDefinition& fd, m_fieldDefinitionList) {
		QString s = fieldDefinitionToSql(fd);
		if (index) {
			form += ",\n";
		}
		++index;
		form += s;
	}

	foreach(const QString& s, m_constraints) {
		if (index) {
			form += ",\n";
		}
		++index;
		form += s;
	}
	form += QString::fromLatin1("\n)");

	if (query.exec(form)) {
		form.clear();
		foreach(const QString& s, m_psql) {
			if (!query.exec(s)) {
				qDebug() << "query builder error : " << query.lastError().text();
				break;
			}
		};
	} else {
		qDebug() << "query builder error : " << query.lastError().text();
	}

	return query;
}

QString WSqlFirebirdTableCreateQueryBuilder::normalize(const QString& fieldName) const
{
	QString normalized = fieldName;
	normalized.remove(QChar('\"'));
	if (m_useQuotedFieldName) {
		normalized = QString("\"%1\"").arg(normalized);
	}
	return normalized;
}

QString WSqlFirebirdTableCreateQueryBuilder::fieldDefinitionToSql(const FieldDefinition& fd) const
{
	QString s = normalize(fd.fieldName);

	s += " ";

	switch (fd.dbType) {
	case Wf::DbSmallInt:
		s += "SMALLINT";
		break;
	case Wf::DbInt:
		s += "INTEGER";
		break;
	case Wf::DbBigInt:
		s += "BIGINT";
		break;
	case Wf::DbFloat:
		s += "FLOAT";
		break;
	case Wf::DbDouble:
		s += "DOUBLE PRECISION";
		break;
	case Wf::DbVarString:
		s += QString("VARCHAR(%1)").arg(fd.length);
		break;
	case Wf::DbVarByte:
		s += "BLOB";
		break;
	case Wf::DbFixString:
		s += QString("CHAR(%1)").arg(fd.length);
		break;
	case Wf::DbDate:
		s += "DATE";
		break;
	default:
		throw WException(QObject::tr("Invalid DB Field Defintion Error."));
		break;
	}
	if (fd.attribute == Wf::DbNotNull) {
		s += " NOT NULL";
	}

	return s;
}
