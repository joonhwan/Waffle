#pragma once

#include "WSqlHeader.h"

class WSqlDatabaseEnabler
{
public:
	WSqlDatabaseEnabler(const QString& dbPath);
	virtual ~WSqlDatabaseEnabler();
	bool initializeDb();
	bool isOpen();
	QSqlError lastError();
protected:
	QString m_dbPath;
};
