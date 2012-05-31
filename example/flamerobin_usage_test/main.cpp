#include "employee.h"
#include "MainWindow.h"
#include "Tables.h"
#include "db/WSqlFirebirdDriver.h"
#include "db/WSqlFirebirdTableBuilder.h"
#include "db/WSqlQueryBuilder.h"
#include "db/WSqlMultiQuery.h"
#include "common/config/WSettingExchanger.h"
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
#include <QSqlRelationalTableModel>
#include <QSqlQueryModel>
#include <QTableView>
#include <iostream>
#include "flamerobin/flamerobin.h"

const char* DBPATH = "c:/db/test.fdb";

void handleError()
{
	QMessageBox::critical(0, QObject::tr("Database error"),
						  QSqlDatabase::database().lastError().text());
}

class WSqlDatabaseEnabler
{
public:
	WSqlDatabaseEnabler()
	{
		initializeDb();
	}
	virtual ~WSqlDatabaseEnabler()
	{
		QSqlDatabase::database().close();
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
} _dbInitor;


int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("firebird_db_test");

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
		WSqlFirebirdTableBuilder tableBuilder(locationTable);
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
		WSqlFirebirdTableBuilder tableBuilder(departmentTable);
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
		WSqlFirebirdTableBuilder tableBuilder(employeeTable);
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

	QList<Employee> employeeList;

	// QSqlRelationalTableModel model;
	// model.setTable("employee");
	// model.setRelation(3, QSqlRelation("LOCATION", "\"id\"", "\"name\""));
	// model.select();
	// for (int i = 0; i < model.rowCount(); ++i) {
	// 	QSqlRecord record = model.record(i);
	// 	WSqlRecordReadAccessor accessor(record);
	// 	WSettingExchanger e(accessor);
	// 	Employee employee;
	// 	employee.exchange(e);
	// 	employeeList << employee;
	// }

	QSqlQuery queryCount(
			"SELECT COUNT(l.\"startdate\") FROM EMPLOYEE l"
		);
	int count = 0;
	if (queryCount.next()) {
		count = queryCount.value(0).toInt();
	}

	// for (int i = 0; i < model.rowCount(); ++i) {
	// 	QSqlRecord record = model.record(i);
	// 	WSqlRecordReadAccessor accessor(record);
	// 	WSettingExchanger e(accessor);
	// 	Employee employee;
	// 	employee.exchange(e);
	// 	employeeList << employee;
	// 	if (i == (model.rowCount() - 1)) {
	// 		if (model.canFetchMore()) {
	// 			model.fetchMore();
	// 		}
	// 	}
	// }
	// Q_ASSERT(count==employeeList.count());

	MainWindow* view = new MainWindow;
	view->setWindowTitle("DB Test");
	view->show();

	return app.exec();
}
