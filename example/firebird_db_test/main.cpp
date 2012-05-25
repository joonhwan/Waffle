#include "MainWindow.h"
#include "db/WSqlFirebirdDriver.h"
#include "db/WSqlQueryBinder.h"
#include "db/WSqlQueryBuilder.h"
#include "db/WSqlFirebirdTableCreateQueryBuilder.h"
#include "db/WSqlTableFieldNameList.h"
#include "db/WSqlTableLister.h"
#include "common/global/WRandom.h"
#include <QApplication>
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
	db.setDatabaseName("c:/db/test.fdb");
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
		if(0 &&
		   !WSqlFirebirdTableCreateQueryBuilder("scooter")
		   .serialPrimaryKey("id")
		   .field("name", "VARCHAR(40)")
		   .field("maxspeed", "INT")
		   .field("maxrange", "INT")
		   .field("weight", "INT")
		   .field("description", "VARCHAR(80)")
		   .query()
		   .isActive()) {
			// break;
		}

		WRandom randomNumber(1950, 2012);

		if (0)
		{
			QSqlDatabase db = QSqlDatabase::database();
			db.transaction();

			QString form =
				QString::fromLatin1("INSERT into scooter (\"name\", \"maxspeed\", \"maxrange\", \"weight\", \"description\") "
									"VALUES (%1, %2, %3, %4, %5)");
			for (int i = 0; i < 100; ++i) {
				int year = randomNumber.generate();
				QString queryString =
					QString(form)
					.arg(QString("'Bike%1'").arg(year))
					.arg(year-900)
					.arg(year-1000)
					.arg(year-1200)
					.arg(QString("'released at the year %1'").arg(year))
					;
				QSqlQuery query(queryString);
				if (!query.isActive()) {
					qDebug() << query.lastError().text();
				}
			}

			db.commit();
		}

		MainWindow* mainWindow = new MainWindow;
		mainWindow->show();

	} while (0);

	do
	{
		break;

		if(!WSqlFirebirdTableCreateQueryBuilder("CD_Library")
		   .serialPrimaryKey("id")
		   .field("artistid", "INT")
		   .field("title", "VARCHAR(255)")
		   .field("year", "INT")
		   .query()
		   .isActive()) {
			handleError();
			// break;
		}

		{
			WRandom randomNumber(1950, 2012);
			for (int i = 0; i < 100; ++i) {
				int year = randomNumber.generate();
				QSqlQuery queryInsert =
					WSqlSimpleInsertQueryBuilder("CD_Library")
					.keyValue("id", i)
					.keyValue("artistid", i+1000)
					.keyValue("title", QString("Living in Korea in %1").arg(year))
					.keyValue("year", year)
					.query();

				if (!queryInsert.isActive()) {
					qDebug() << queryInsert.lastError().text();
				}
			}
		}

		std::cout << WSqlTableFieldNameList("CD_Library") << std::endl;
		std::cout << "------------------" << std::endl;
		std::cout << WSqlTableLister("CD_Library", 10) << std::endl;

		std::cout << "removing table..." << std::endl;
		{
			QSqlQuery queryRemove("DROP TABLE CD_Library");
			if (!queryRemove.isActive()) {
				handleError();
			}
		}

		std::cout << "------------------" << std::endl;
		std::cout << WSqlTableLister("CD_Library") << std::endl;

		// {
		// 	QSqlQuery query2("SELECT * FROM CD_Library");
		// 	QSqlRecord column = query2.record();
		// 	int columnCount = column.count();
		// 	while (query2.next()) {
		// 		for (int i = 0; i < columnCount; ++i) {
		// 			if (i!=0) {
		// 				std::cout << ", ";
		// 			}
		// 			std::cout << query2.value(i).toString().toAscii().data();
		// 		}
		// 	}
		// }
	} while (0);


	if (0)
	{
		QSqlQuery query(QString("SELECT DISTINCT A.*,GROUP_ID FROM (SELECT FIRST %1 "
								"SKIP 0 * FROM GLASS ORDER BY \"END\" DESC) AS A "
								"LEFT JOIN MASK ON A.SID=GLASS_SID WHERE glass_id NOT LIKE 'DD%%' ")
						.arg(100));

		int columnCount = 5;
		QSqlRecord columnInfo = query.record();
		std::cout << std::endl;
		columnCount = columnInfo.count();
		for (int i = 0; i < columnCount; ++i) {
			if (i!=0) {
				std::cout << ", ";
			}
			std::cout << columnInfo.fieldName(i).toAscii().data();
		}
		std::cout << std::endl << " ----------------------------------- " << std::endl;

		bool first = true;

		while (query.next()) {
			for (int i = 0; i < columnCount; ++i) {
				if (i!=0) {
					std::cout << ", ";
				}
				std::cout << query.value(i).toString().toAscii().data();
			}
			std::cout << std::endl;
			// qDebug() << query.value(0) << ", " << query.value(1);
			// QMapIterator<QString, QVariant> it(query.boundValues());
			// while (it.hasNext()) {
			// 	it.next();
			// 	qDebug() << it.key() << " : " << it.value();
			// 	// std::	std::cout << i.key().toAscii().data() << ": "
			// 	// 		  << i.value().toString().toAscii().data() << std::std::endl;
			// }
			// qDebug() << "-----";
		}
	}

	QSqlDatabase::database().close();
	return app.exec();
}
