#include "db/WSqlFirebirdDriver.h"
#include "db/WSqlFirebirdTableBuilder.h"
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
	WSqlFirebirdTableBuilder tableBuilder("EMPLOYEE");

	tableBuilder
		.serialPrimaryKey("id")
		.field("name", "varchar(40)", Wf::DbNotNull, "noname")
		.foreignSerialKey("departmentid", "DEPARTMENT", "id")
		.field("extension", "integer", Wf::DbNotNull)
		.field("email", "varchar(40)", Wf::DbNotNull)
		.field("startdate", "date", Wf::DbNotNull);

	QString updateQuery;
	if (tableBuilder.tryGetUpdateQueryString(updateQuery)) {
	}

	tableBuilder.test();

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
