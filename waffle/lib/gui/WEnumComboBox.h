#pragma once

#include <QComboBox>
#include <QMetaEnum>

struct QMetaObject;

// though c++ enum should not be thought as a plain 'integer' type.....
// it is just convenient to assume that...(like in QtPropertyBrowser framework).
class WEnumComboBox : public QComboBox
{
    Q_OBJECT
public:
    WEnumComboBox(QWidget* parent=0);
	WEnumComboBox(const QMetaObject& metaObject, const char* enumName, QWidget* parent=0);
	WEnumComboBox(QMetaEnum metaEnum, QWidget* parent=0);
    virtual ~WEnumComboBox();
	void addItems(const QMetaObject& metaObj, const char* enumName);
	void addItems(QMetaEnum metaEnum);
	int currentEnumInt() const;	// return enum value in 'integer' type.
	void setCurrentEnumInt(int value); // select index whose enum value is...
protected:
	QMetaEnum m_metaEnum;
};
