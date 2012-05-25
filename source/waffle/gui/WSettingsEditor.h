#pragma once

#include "gui/WSettingEditInfo.h"
#include <QMap>
#include <QSet>
#include <QSettings>
#include <QWidget>

class QSettings;
class QtAbstractPropertyBrowser;
class QtGroupPropertyManager;
class QtProperty;
class QtVariantEditorFactory;
class QtVariantProperty;
class QtVariantPropertyManager;
class QVariant;
class WSettingExchangeable;

class WSettingsEditor : public QWidget
{
    Q_OBJECT
public:
	enum Type
	{
		Tree = 0,
		GroupBox,
		Button,
	};
	WSettingsEditor(QSettings& settings,
					const QString& title = QLatin1String("Settings Editor"),
					Type editorType = Tree,
					QWidget* parent=0);
    virtual ~WSettingsEditor();
signals:
private slots:
	void valueChanged(QtProperty* property, const QVariant& value);
protected:
	QString m_title;
	Type m_editorType;
	QSettings& m_settings;
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
	QSet<QString> m_arrayPropertyKeys;

	void generateUi(QSettings& settings);
	QtVariantProperty* addGroup(const QString& id);
	QtVariantProperty* addProperty(const QString& key,
								   const QVariant& value);
};
