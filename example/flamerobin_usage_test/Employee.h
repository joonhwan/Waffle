#pragma once

#include "common/config/WSettingAccessor.h"
#include "common/config/WSettingExchangeable.h"
#include <QString>
#include <QDate>
#include <QSqlRecord>
#include <QVariant>

class WSqlRecordReadAccessor : public WSettingAccessor
{
public:
	WSqlRecordReadAccessor(QSqlRecord& record);
	virtual bool hasKeyStartsWith(const QString& key);
	virtual void beginGroup(const QString& key);
	virtual void endGroup();
	virtual int beginArray(const QString& key);
	virtual void beginArrayIndex(int index);
	virtual void endArrayIndex();
	virtual void endArray();
	virtual QVariant value(const QString& key, const QVariant& defaultValue);
	virtual void setValue(const QString& key, const QVariant& value);
protected:
	QSqlRecord m_record;
};

class Employee : public WSettingExchangeable
{
public:
	QString name;
	QString department;
	int extension;
	QString email;
	QDate startDate;
	int age;
	virtual void exchange(WSettingExchanger& e);
};




