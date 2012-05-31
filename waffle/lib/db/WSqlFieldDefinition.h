#pragma once

#include "db/WSqlType.h"
#include <QString>
#include <QSharedData>
#include <QSharedDataPointer>

class WSqlFieldDefinitionData : public QSharedData
{
public:
	WSqlFieldDefinitionData()
		: attribute(0)
	{
	}
	QString tableName; // redundant, but for handling in
					   // WSqlRelationalTableModel
	QString fieldName;
	QString displayName;
	QString dataType;
	int attribute;
	QString defaultValue;
	QVariantMap properties;
};

class WSqlFieldDefinition
{
public:
    WSqlFieldDefinition()
	{
		d = new WSqlFieldDefinitionData;
	}
	WSqlFieldDefinition(const QString& fieldName,
						const QString& dataType,
						int attribute,
						const QString& defaultValue = QString())
	{
		d = new WSqlFieldDefinitionData;
		d->fieldName = fieldName;
		d->dataType = dataType;
		d->attribute = attribute;
		d->defaultValue = defaultValue;
	}
	WSqlFieldDefinition(const WSqlFieldDefinition& other)
		: d(other.d)
	{
	}
    virtual ~WSqlFieldDefinition()
	{
	}
	bool isNormalField() const {
		return
			!isPrimaryKey() && !isForeignKey();
	}
	bool isPrimaryKey() const {
		int v = d->attribute;
		return (v & Wf::DbPrimaryKey) == Wf::DbPrimaryKey;
	}
	bool isForeignKey() const {
		return (d->attribute & Wf::DbForeignKey) == Wf::DbForeignKey;
	}
	bool hasDefaultValue() const {
		return (d->attribute & Wf::DbHasDefault) == Wf::DbHasDefault;
	}
	bool isNotNull() const {
		return (d->attribute & Wf::DbNotNull) == Wf::DbNotNull;
	}
	bool isAutoIncrement() const {
		return (d->attribute & Wf::DbAutoIncrement) == Wf::DbAutoIncrement;
	}
	void setTableName(const QString& tableName) {
		d->tableName = tableName;
	}
	void setFieldName(const QString& fieldName) {
		d->fieldName = fieldName;
	}
	void setDisplayName(const QString& displayName) {
		d->displayName = displayName;
	}
	void setDataType(const QString& dataType) {
		d->dataType = dataType;
	}
	void setAttribute(int attribute) {
		d->attribute = attribute;
	}
	void addAttribute(int attribute) {
		d->attribute |= attribute;
	}
	void setDefaultValue(const QString& defaultValue) {
		d->defaultValue = defaultValue;
	}
	void setProperty(const QString& key, const QVariant& value) {
		d->properties[key] = value;
	}
	const QString& tableName() const {
		return d->tableName;
	}
	const QString& fieldName() const {
		return d->fieldName;
	}
	const QString& displayName() const {
		if (d->displayName.isEmpty()) {
			return d->fieldName;
		} else {
			return d->displayName;
		}
	}
	const QString& dataType() const {
		return d->dataType;
	}
	const int attribute() const {
		return d->attribute;
	}
	const QString& defaultValue() const {
		return d->defaultValue;
	}
	QVariant property(const QString& key) const {
		QVariant v;
		if (d->properties.contains(key)) {
			v = d->properties[key];
		}
		return v;
	}
	const QVariantMap& properties() const {
		return d->properties;
	}
	bool operator==(const WSqlFieldDefinition& rhs) const {
		return d->tableName == rhs.d->tableName
			&& d->fieldName == rhs.d->fieldName
			&& d->dataType == rhs.d->dataType
			&& d->attribute == rhs.d->attribute
			&& d->defaultValue == rhs.d->defaultValue
			&& d->properties == rhs.d->properties;
	}
	bool operator!=(const WSqlFieldDefinition& rhs) const {
		return !((*this)==rhs);
	}
private:
	QSharedDataPointer<WSqlFieldDefinitionData> d;
};

typedef QList<WSqlFieldDefinition> WSqlFieldDefinitionList;
