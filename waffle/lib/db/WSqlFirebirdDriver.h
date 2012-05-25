#pragma once

#include "firebird/QIBaseDriver.h"

class WSqlFirebirdDriver : public QIBaseDriver
{
    Q_OBJECT
public:
    explicit WSqlFirebirdDriver(QObject *parent = 0);
    virtual ~WSqlFirebirdDriver();
	virtual bool open(const QString & db,
					  const QString & user,
					  const QString & password,
					  const QString & host,
					  int port,
					  const QString & connOpts);
    virtual bool open(const QString & db,
					  const QString & user,
					  const QString & password,
					  const QString & host,
					  int port) {
		return open (db, user, password, host, port, QString());
	}
	void setNewDatabasePageSize(int pageSize) {
		m_newDatabasePageSize = pageSize;
	}
signals:
public slots:
protected:
	bool openLocal(const QString& db,
				   const QString& user,
				   const QString& password,
				   int port,
				   const QString& connOpts);
	bool createLocalDbFile(const QString& dbPath, int pageSize=8096);
	bool ensureServerRunning(const char* serviceName);
private:
	int m_newDatabasePageSize;
};

QString wFirebirdDataType(QVariant::Type type);

