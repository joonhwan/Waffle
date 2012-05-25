#pragma once

// Firebird metatable에 대한 학습이 안끝난 관계로
// 일부의 정보만 얻을 수 있는 상태.
// 따라서 여기 정의되어 있는 것만 사용?
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

