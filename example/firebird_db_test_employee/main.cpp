#include "MainWindow.h"
#include "db/WSqlFirebirdDriver.h"
#include "db/WSqlQueryBinder.h"
#include "db/WSqlQueryBuilder.h"
#include "db/WSqlFirebirdTableCreateQueryBuilder.h"
#include "db/WSqlTableFieldNameList.h"
#include "db/WSqlTableLister.h"
#include "common/global/WRandom.h"
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

	if (!initializeDb()) {
		return -1;
	}

	do
	{
		{
			QSqlQuery query = WSqlFirebirdTableCreateQueryBuilder("location")
							  .serialPrimaryKey("id")
							  .field("name", Wf::DbVarString, 40)
							  .query();
			if(!query.isActive()) {
				qDebug() << query.lastError().text();
			}

			std::cout << WSqlTableFieldNameList("location") << std::endl;
		}

		{
			QSqlQuery query = WSqlFirebirdTableCreateQueryBuilder("department")
							  .serialPrimaryKey("id")
							  .field("name", Wf::DbVarString, 40)
							  .foreignSerialKey("locationid", "location", "id")
							  .query();
			if(!query.isActive()) {
				qDebug() << query.lastError().text();
			}

			std::cout << WSqlTableFieldNameList("department") << std::endl;
		}

		{
			QSqlQuery query  = WSqlFirebirdTableCreateQueryBuilder("employee")
							   .serialPrimaryKey("id")
							   .field("name", Wf::DbVarString, 40)
							   .foreignSerialKey("departmentid", "department", "id")
							   .field("extension", Wf::DbInt)
							   .field("email", Wf::DbVarString, 40)
							   .field("startdate", Wf::DbDate)
							   .query();
			if (!query.isActive()) {
				qDebug() << query.lastError().text();
			}

			std::cout << WSqlTableFieldNameList("employee") << std::endl;
		}

		QSqlDatabase db = QSqlDatabase::database();

		db.transaction();
		WRandom randomNumber(1950, 2012);
		for (int i = 0; i < 10; ++i) {
			QSqlQuery query =
				WSqlSimpleInsertQueryBuilder("location")
				.keyValue("name", QString("area%1").arg(i))
				.query();
			if (!query.isActive()) {
				qDebug() << query.lastError().text();
			}
		}
		db.commit();

		WRandom randomLocation(1, 8);

		db.transaction();
		for (int i = 0; i < 100; ++i) {
			QSqlQuery query =
				WSqlSimpleInsertQueryBuilder("department")
				.keyValue("name", QString("dept%1").arg(i))
				.keyValue("locationid", randomLocation.generate())
				.query();
			if (!query.isActive()) {
				qDebug() << query.lastError().text();
			}
		}
		db.commit();

		WRandom randomDept(1, 99);
		db.transaction();
		for (int i = 0; i < 1000; ++i) {
			QSqlQuery query =
				WSqlSimpleInsertQueryBuilder("employee")
				.keyValue("name", QString("employe%1").arg(i))
				.keyValue("departmentid", randomDept.generate())
				.keyValue("extension", randomNumber.generate())
				.keyValue("email", QString("acount_%1@yahoo.com").arg(i))
				.keyValue("startdate", QDate::currentDate())
				.query();
			if (!query.isActive()) {
				qDebug() << query.lastError().text();
			}
		}
		db.commit();

		MainWindow* mainWindow = new MainWindow;
		mainWindow->show();

	} while (0);

	QSqlDatabase::database().close();

	QFile(DBPATH).remove();

	return app.exec();
}
