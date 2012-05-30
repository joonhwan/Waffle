#include "db/WSqlFirebirdDriver.h"
#include "db/WSqlFirebirdTableBuilder.h"
#include "db/WSqlMultiQuery.h"
#include "flamerobin/sql/multistatement.h"
#include <QApplication>
#include <QDate>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QMapIterator>
#include <QVariant>
#include <iostream>
#include "flamerobin/flamerobin.h"

const char* DBPATH = "c:/db/test.fdb";

void handleError()
{
	QMessageBox::critical(0, QObject::tr("Database error"),
						  QSqlDatabase::database().lastError().text());
}

bool initializeDb()
{
	WSqlFirebirdDriver* driver = new WSqlFirebirdDriver;
	driver->setNewDatabasePageSize(1024*8);
	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	// db.setHostName("192.168.0.82");
	db.setHostName("localhost");
	db.setDatabaseName(DBPATH);
	db.setUserName("SYSDBA");
	db.setPassword("masterkey");
	// db.setConnectOptions(
	// 		QString("ISC_DPB_LC_CTYPE=%1;ISC_DPB_SQL_ROLE_NAME=%2")
	// 		.arg(encoding)
	// 		.arg(role));

	if (!db.open()) {
		handleError();
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("firebird_db_test");

	initializeDb();
	;
	// {
	// 	QStringList queryList = QStringList()
	// 		<< (QStringList()
	// 			<< "CREATE TABLE LOCATION"
	// 			<< "("
	// 			<< "\"id\" bigint NOT NULL,"
	// 			<< "\"name\" varchar(40) DEFAULT 'noname' NOT NULL,"
	// 			<< "    CONSTRAINT PK_LOCATION PRIMARY KEY (\"id\")"
	// 			<< ");").join(QLatin1String("\n"))
	// 		<< "CREATE GENERATOR GEN_ID_OF_LOCATION;"
	// 		<< (QStringList()
	// 			<< "CREATE TRIGGER TRIG_ID_OF_LOCATION_BI FOR LOCATION ACTIVE"
	// 			<< "BEFORE INSERT POSITION 0"
	// 			<< "AS BEGIN"
	// 			<< "IF (NEW.\"id\" IS NULL) THEN"
	// 			<< "NEW.\"id\" = GEN_ID(\"GEN_ID_OF_LOCATION\", 1);"
	// 			<< "END").join(QLatin1String("\n"));
	// 	foreach(const QString& query, queryList) {
	// 		QSqlQuery q(query);
	// 		if (!q.isActive()) {
	// 			qDebug() << "query error : " << q.lastError().text();
	// 		}
	// 	}
	// }
	// {
	// 	QString query = 
	// 		(QStringList()
	// 		 << "CREATE TABLE LOCATION"
	// 		 << "("
	// 		 << "\"id\" bigint NOT NULL,"
	// 		 << "\"name\" varchar(40) DEFAULT 'noname' NOT NULL,"
	// 		 << "    CONSTRAINT PK_LOCATION PRIMARY KEY (\"id\")"
	// 		 << ");"
	// 		 << ""
	// 		 << "CREATE GENERATOR GEN_ID_OF_LOCATION;"
	// 		 << ""
	// 		 << "SET TERM ^ ;"
	// 		 << "CREATE TRIGGER TRIG_ID_OF_LOCATION_BI FOR LOCATION ACTIVE"
	// 		 << "BEFORE INSERT POSITION 0"
	// 		 << "AS BEGIN"
	// 		 << "IF (NEW.\"id\" IS NULL) THEN"
	// 		 << "NEW.\"id\" = GEN_ID(\"GEN_ID_OF_LOCATION\", 1);"
	// 		 << "END^"
	// 		 << ""
	// 		 << "SET TERM ; ^"
	// 			).join(QLatin1String("\n"));
	// 	fr::MultiStatement ms(query);
	// 	while (1) {
	// 		fr::SingleStatement ss = ms.getNextStatement();
	// 		if (!ss.isValid()) {
	// 			break;
	// 		}
	// 		qDebug() << "-----";
	// 		qDebug() << ss.getSql();
	// 	}
	// }
	{
		WSqlFirebirdTableBuilder tableBuilder("LOCATION");
		tableBuilder
			.serialPrimaryKey("id")
			.field("name", "varchar(40)", Wf::DbNotNull, "noname");
		QStringList updateQuery;
		if (tableBuilder.tryGetUpdateQueryString(updateQuery)) {
			WSqlMultiQuery query(updateQuery);
			if (!query.isActive()) {
				qDebug() << "query error : " << query.lastError().text();
				qDebug() << "failed query ...\n" << updateQuery;
			}
		} else {
			qDebug() << tableBuilder.lastError();
		}
	}
	{
		WSqlFirebirdTableBuilder tableBuilder("DEPARTMENT");
		tableBuilder
			.serialPrimaryKey("id")
			.field("name", "varchar(40)", Wf::DbNotNull, "noname")
			.foreignSerialKey("location_id", "LOCATION", "id");
		QStringList updateQuery;
		if (tableBuilder.tryGetUpdateQueryString(updateQuery)) {
			WSqlMultiQuery query(updateQuery);
			if (!query.isActive()) {
				qDebug() << "query error : " << query.lastError().text();
				qDebug() << "failed query ...\n" << updateQuery;
			}
		} else {
			qDebug() << tableBuilder.lastError();
		}
	}
	{
		WSqlFirebirdTableBuilder tableBuilder("EMPLOYEE");
		tableBuilder
			.serialPrimaryKey("id")
			.field("name", "varchar(40)", Wf::DbNotNull, "noname")
			.foreignSerialKey("department_id", "DEPARTMENT", "id")
			.field("extension", "integer", Wf::DbNotNull)
			.field("email", "varchar(40)", Wf::DbNotNull)
			.field("startdate", "date", Wf::DbNotNull)
			.field("age", "integer", Wf::DbNotNull);

		QStringList updateQuery;
		if (tableBuilder.tryGetUpdateQueryString(updateQuery)) {
			WSqlMultiQuery query(updateQuery);
			if (!query.isActive()) {
				qDebug() << "query error : " << query.lastError().text();
				qDebug() << "failed query ...\n" << updateQuery;
			}
		} else {
			qDebug() << tableBuilder.lastError();
		}
	}

	// tableBuilder.test();

	// fr::DatabasePtr db(new fr::Database());
	// {
	// 	fr::RootPtr root(new fr::Root());
	// 	fr::ServerPtr s(new fr::Server());
	// 	s->setName_(QLatin1String("Localhost"));
	// 	s->setHostname(QLatin1String("localhost"));
	// 	root->addServer(s);

	// 	db->setServer(s);
	// 	db->setUsername(QString::fromLatin1("SYSDBA"));
	// 	db->setRawPassword(QString::fromLatin1("masterkey"));
	// 	db->setPath(QString::fromLatin1("c:/db/test.fdb"));
	// 	db->connect(QString::fromLatin1("masterkey"));
	// }

	// if (!db->isConnected()) {
	// 	qDebug() << "unable to connect to database.";
	// 	return -1;
	// }

	// return app.exec();

	QSqlDatabase::database().close();

	return 0;
}
