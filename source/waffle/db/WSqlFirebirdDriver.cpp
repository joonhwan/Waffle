#include "WSqlFirebirdDriver.h"
#include <QSqlError> // #include <qsqlerror.h>
#include <QFileInfo>
#include <QDir>
#include <shlwapi.h>
#include <QDebug>

const char* FB_SERVER_NAME = "FirebirdServerDefaultInstance";
const char* FB_SERVER_NAME_2 = "FirebirdGuardianDefaultInstance";

//explicit
WSqlFirebirdDriver::WSqlFirebirdDriver(QObject *parent)
	: QIBaseDriver(parent)
	, m_newDatabasePageSize(16*1024)
{
}

//virtual
WSqlFirebirdDriver::~WSqlFirebirdDriver()
{
}

//virtual
bool WSqlFirebirdDriver::open(const QString & db,
							  const QString & user,
							  const QString & password,
							  const QString & host,
							  int port,
							  const QString & connOpts)
{
	if (host.toLower()=="localhost" ||
		host.toLower()=="127.0.0.1") {
		return openLocal(db, user, password, port, connOpts);
	} else {
		return QIBaseDriver::open(db, user, password, host, port, connOpts);
	}
}

bool WSqlFirebirdDriver::openLocal(const QString& db,
								   const QString& user,
								   const QString& password,
								   int port,
								   const QString& connOpts)
{
	bool fileOk = false;

	do
	{
		if (isOpen()) {
			close();
		}

		// if (!ensureServerRunning(FB_SERVER_NAME)
		// 	|| !ensureServerRunning(FB_SERVER_NAME_2)) {
		// 	setLastError(QSqlError(QString::fromLatin1("unable to run firebird server service")));
		// 	break;
		// }

		QFileInfo fileInfo(db);
		if (!fileInfo.exists()) {
			QString dbPath = fileInfo.absoluteFilePath();
			QDir dir = fileInfo.absoluteDir();
			if (!dir.mkpath(dir.path())) {
				setLastError(
						QSqlError(QString::fromLatin1("unable to create database dir at %1.")
								  .arg(dir.path()))
					);
				break;
			}
			if (!createLocalDbFile(dbPath, m_newDatabasePageSize)) {
				setLastError(
						QSqlError(QString::fromLatin1("unable to create database file at %1.")
								  .arg(dbPath)));
				break;
			}
		}

		// file itself sometimes is created in a delay(asynchronously created)
		// Q_ASSERT(fileInfo.exists());

		fileOk = true;
	} while (0);

	if (fileOk) {
		return QIBaseDriver::open(db, user, password, QLatin1String("localhost"), port, connOpts);
	} else {
		return false;
	}
}

bool WSqlFirebirdDriver::createLocalDbFile(const QString& dbPath, int pageSize)
{
	QString normDbPath = dbPath;
	normDbPath.replace("\\", "/");

	bool created = false;
	QString query = QString("CREATE DATABASE \"%1\" "
							"USER \"SYSDBA\" PASSWORD \"masterkey\" page_size %2")
					.arg(normDbPath)
					.arg(pageSize);

	ISC_STATUS status[20];
	::memset(&status[0], 0, sizeof (status));
	isc_db_handle idh = 0;
	isc_tr_handle ith = 0;
	// ISC_STATUS ISC_EXPORT isc_dsql_execute_immediate(ISC_STATUS*,
	// 												 isc_db_handle*,
	// 												 isc_tr_handle*,
	// 												 unsigned short,
	// 												 const ISC_SCHAR*,
	// 												 unsigned short,
	// 												 XSQLDA*);
	QByteArray queryP = query.toLocal8Bit();
	const short dialect = 3;
	isc_dsql_execute_immediate(status, &idh, &ith, 0,
							   queryP.data(),
							   dialect, 0);

	char errorMessage[512] = "";
	if (status[0]==1 && status[1]) {
		// following routine from 'Interbase 6 API Guide" output wired one.
		// 'isc_interprete' seems to be more correct?
		// long SQLCODE = isc_sqlcode(status);
		// isc_sql_interprete(SQLCODE, errorMessage, sizeof(errorMessage));
#pragma warning(push)
#pragma warning(disable : 4996)
		ISC_STATUS *pvector = status;
		isc_interprete(errorMessage, &pvector);
#pragma warning(pop)
	} else {
		created = true;
	}

	return created;
}

bool WSqlFirebirdDriver::ensureServerRunning(const char* serviceName)
{
	bool ensured = false;
	SC_HANDLE scManager = 0;
	SC_HANDLE service = 0;
	do
	{
		// Open SCM
		scManager = OpenSCManager(0, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
		if (!scManager) {
			qDebug() << "OpenSCManager() failed (error code=" << GetLastError() << ")";
			break;
		}

		// Open Service
		service = OpenServiceA(scManager, serviceName, SERVICE_ALL_ACCESS);
		if (!service) {
			break;
		}

		// Change Service Type
		if (!ChangeServiceConfigA(service, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE,
								  0, 0, 0, 0, 0, 0, 0)) {
			break;
		}

		// Start Service
		if (!StartService(service, 0, 0)) {
			if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
				break;
			}
		}

		ensured = true;

	} while (0);

	if ( service ) {
		CloseServiceHandle(service);
	}
	if ( scManager ) {
		CloseServiceHandle(scManager);
	}
	return ensured;
}
