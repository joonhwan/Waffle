#include "Employee.h"
#include "common/config/WSettingExchanger.h"

// //virtual
// void EmployeeTable::exchange(WSettingExchanger& e)
// {
// 	e.handle(m_employeeList, "employees", EmployeeList());
// }

WSqlRecordReadAccessor::WSqlRecordReadAccessor(QSqlRecord& record)
	: WSettingAccessor(WSettingAccessor::Read)
	, m_record(record)
{
}

//virtual
bool WSqlRecordReadAccessor::hasKeyStartsWith(const QString& key)
{
	// should not be called?
	Q_ASSERT(FALSE);
	return false;
}

//virtual
void WSqlRecordReadAccessor::beginGroup(const QString& key)
{
	// should not be called?
	Q_ASSERT(FALSE);
}

//virtual
void WSqlRecordReadAccessor::endGroup()
{
	// should not be called?
	Q_ASSERT(FALSE);
}

//virtual
int WSqlRecordReadAccessor::beginArray(const QString& key)
{
	// should not be called?
	Q_ASSERT(FALSE);
	return 0;
}

//virtual
void WSqlRecordReadAccessor::beginArrayIndex(int index)
{
	// should not be called?
	Q_ASSERT(FALSE);
}

//virtual
void WSqlRecordReadAccessor::endArrayIndex()
{
	// should not be called?
	Q_ASSERT(FALSE);
}

//virtual
void WSqlRecordReadAccessor::endArray()
{
	// should not be called?
	Q_ASSERT(FALSE);
}

//virtual
QVariant WSqlRecordReadAccessor::value(const QString& key, const QVariant& defaultValue)
{
	QVariant v = m_record.value(key);
	if (!v.isValid()) {
		v = defaultValue;
	}
	return v;
}

//virtual
void WSqlRecordReadAccessor::setValue(const QString& key, const QVariant& value)
{
	// should not be called.
	Q_ASSERT(FALSE);
}

//virtual
void Employee::exchange(WSettingExchanger& e)
{
	e.handle(name, "name", name);
	e.handle(department, "department", department);
	e.handle(extension, "extension", extension);
	e.handle(email, "email", email);
	e.handle(startDate, "startdate", startDate);
	e.handle(age, "age", age);
}


