#include "WEnumComboBox.h"
#include <QMetaObject>

WEnumComboBox::WEnumComboBox(QWidget* parent)
	: QComboBox(parent)
{
}

WEnumComboBox::WEnumComboBox(const QMetaObject& metaObject, const char* enumName, QWidget* parent)
	: QComboBox(parent)
{
	this->addItems(metaObject, enumName);
}


WEnumComboBox::WEnumComboBox(QMetaEnum metaEnum, QWidget* parent)
	: QComboBox(parent)
{
	this->addItems(metaEnum);
}

//virtual
WEnumComboBox::~WEnumComboBox()
{
}

void WEnumComboBox::addItems(const QMetaObject& metaObj, const char* enumName)
{
	int index = metaObj.indexOfEnumerator(enumName);
	Q_ASSERT(index >= 0);
	if (index >= 0) {
		this->addItems(metaObj.enumerator(index));
	}
}

void WEnumComboBox::addItems(QMetaEnum metaEnum)
{
	clear();
	if (metaEnum.isValid()) {
		for (int i = 0; i < metaEnum.keyCount(); ++i) {
			this->addItem(QString::fromLatin1(metaEnum.key(i)),
						  metaEnum.value(i));
		}
	}
}

int WEnumComboBox::currentEnumInt() const
{
	int e = 0;
	int index = this->currentIndex();
	if (index >= 0) {
		e = this->itemData(index).toInt();
	}
	return e;
}

void WEnumComboBox::setCurrentEnumInt(int value)
{
	int index = this->findData(value);
	if (index >= 0) {
		this->setCurrentIndex(index);
	}
}
