#pragma once

#include "common/config/WSettingAccessor.h"
#include "gui/WSettingEditInfo.h"
#include <QMap>
#include <QSet>
#include <QVariant>
#include <QWidget>

class QSettings;
class QtAbstractPropertyBrowser;
class QtGroupPropertyManager;
class QtProperty;
class QtVariantEditorFactory;
class QtVariantProperty;
class QtVariantPropertyManager;
class WSettingExchangeable;

class WSettingExchangeableEditor : public QWidget
								 , public WSettingAccessor
{
    Q_OBJECT
public:
	enum Type
	{
		Tree = 0,
		GroupBox,
		Button,
	};
    WSettingExchangeableEditor(WSettingExchangeable& exchangeable,
							   const QString& title = QLatin1String("Settings Editor"),
							   Type editorType = Tree,
							   QWidget* parent=0);
    virtual ~WSettingExchangeableEditor();
	// WSettingAccessor interfaces
	virtual bool hasKeyStartsWith(const QString& key);
	virtual void beginGroup(const QString& key);
	virtual void endGroup();
	virtual int beginArray(const QString& key);
	virtual void beginArrayIndex(int index);
	virtual void endArrayIndex();
	virtual void endArray();
	virtual QVariant value(const QString& key, const QVariant& defaultValue);
	virtual void setValue(const QString& key, const QVariant& value);
	virtual void setEnumListHint(const QString& key, const QStringList& enumList);
signals:
	void modified();
	void restored();
	void saved();
public slots:
	void save();
	void restore();
private slots:
	void valueChanged(QtProperty* property, const QVariant& value);
protected:
	WSettingExchangeable& m_exchangeable;
	bool m_dirty;
	QString m_title;
	Type m_editorType;
	QtGroupPropertyManager* m_groupManager;
	QtVariantPropertyManager* m_variantManager;
	QtVariantEditorFactory* m_variantFactory;
	QtAbstractPropertyBrowser* m_propertyEditor;
	QtVariantProperty* m_topProperty;
	WSettingsEditInfo m_editorInfo;
	QMap<QtProperty*, QString> m_propertyToId;
    QMap<QString, QtVariantProperty*> m_idToProperty;
	QMap<QString, QtVariantProperty*> m_idToGroup;
	QMap<QString, QString> m_normalizedKeyToOriginalKey;
	QMap<QString, int> m_arrayCountHint;
	QSet<QString> m_arrayPropertyKeys;

	QStringList m_currentBasePath;
	QList<QtVariantProperty*> m_currentParentProperty;

	void generateUi(WSettingExchangeable& settings);
	QtVariantProperty* addGroup(const QString& id);
	QtVariantProperty* addEnum(const QString& key,
							   const QStringList& enumNames);
	QtVariantProperty* addProperty(const QString& key,
								   const QVariant& value);
};
