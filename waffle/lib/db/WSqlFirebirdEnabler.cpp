#include "WSqlFirebirdEnabler.h"
#include "WSqlFirebirdDriver.h"

WSqlDatabaseEnabler::WSqlDatabaseEnabler(const QString& dbPath)
	: m_dbPath(dbPath)
{
	initializeDb();
}

//virtual
WSqlDatabaseEnabler::~WSqlDatabaseEnabler()
{
	QSqlDatabase::database().close();
}

bool WSqlDatabaseEnabler::initializeDb()
{
	WSqlFirebirdDriver* driver = new WSqlFirebirdDriver;
	driver->setNewDatabasePageSize(1024*8);
	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	// db.setHostName("192.168.0.82");
	db.setHostName("localhost");
	db.setDatabaseName(m_dbPath);
	db.setUserName("SYSDBA");
	db.setPassword("masterkey");
	// db.setConnectOptions(
	// 		QString("ISC_DPB_LC_CTYPE=%1;ISC_DPB_SQL_ROLE_NAME=%2")
	// 		.arg(encoding)
	// 		.arg(role));
	if (!db.open()) {
		return false;
	}
	return true;
}

bool WSqlDatabaseEnabler::isOpen()
{
	return QSqlDatabase::database().isOpen();
}

QSqlError WSqlDatabaseEnabler::lastError()
{
	return QSqlDatabase::database().lastError();
}

