#pragma once

// Firebird metatable�� ���� �н��� �ȳ��� �����
// �Ϻ��� ������ ���� �� �ִ� ����.
// ���� ���� ���ǵǾ� �ִ� �͸� ���?
class WSqlFirebirdFieldDefine
{
public:
	enum Type {
		FLOAT,
		DOUBLE,
		INTEGER,
		SMALLINT,
		VARCHAR,
		CHAR,
	};
	bool optional() const;
	QString name() const;
	QString format() const;
	Type type() const;
	QString name;
	QString description;
	QVariant defaultValue;
	int notNullConstraint;
	int length;
	QVariant precision;
	QString type;
};

class WSqlTableDefine
{
public:
	WSqlTableInfo(const QString& tableName, QSqlDatabase db = QSqlDatabase());
	virtual ~WSqlTableInfo();
	bool valid() const;
	QString fieldDefine(
protected:
};

