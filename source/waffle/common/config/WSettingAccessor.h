#pragma once

class QString;
class QVariant;
class QStringList;

class WSettingAccessor
{
public:
	enum Type
	{
		Read,
		Write,
	};
	WSettingAccessor(Type type)
		: m_type(type)
	{
	}
	virtual bool hasKeyStartsWith(const QString& key) = 0;
	virtual void beginGroup(const QString& key) = 0;
	virtual void endGroup() = 0;
	virtual int beginArray(const QString& key) = 0;
	virtual void beginArrayIndex(int index) = 0;
	virtual void endArrayIndex() = 0;
	virtual void endArray() = 0;
	virtual QVariant value(const QString& key, const QVariant& defaultValue) = 0;
	virtual void setValue(const QString& key, const QVariant& value) = 0;
	virtual void setEnumListHint(const QString& key, const QStringList& enumList) {
		// nothing to do by default
	}
	bool isReading() const {
		return m_type==Read;
	}
protected:
	Type m_type;
};
