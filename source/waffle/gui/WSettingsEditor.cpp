#include "WSettingsEditor.h"
#include "QtPropertyBrowser/headers.h"
#include <QHBoxLayout>

namespace {

void split(const QString& key, QString& parentKey, QString& childKey)
{
	int index = key.lastIndexOf(QLatin1Char('/'));
	if (index>0) {
		parentKey = key.left(index);
		childKey = key.right(key.length()-index-1);
	} else {
		parentKey.clear();
		childKey = key;
	}
}

}

QString WSettingsEditInfo::guiName(const QString& settingKey) const
{
	QString v = settingKey;
	if (m_infos.contains(settingKey)) {
		v = m_infos[settingKey].guiName;
	} else {
		QString parentKey;
		split(settingKey, parentKey, v);
	}
	return v;
}

QString WSettingsEditInfo::guiToolTip(const QString& settingKey) const
{
	QString v = settingKey;
	if (m_infos.contains(settingKey)) {
		v = m_infos[settingKey].guiName;
	}
	return v;
}

WSettingsEditor::WSettingsEditor(QSettings& settings,
								 const QString& title,
								 Type editorType,
								 QWidget* parent)
	: QWidget(parent)
	, m_settings(settings)
	, m_editorType(editorType)
	, m_title(title)
	, m_topProperty(0)
{
	m_groupManager = new QtGroupPropertyManager(this);
	m_variantManager = new QtVariantPropertyManager(this);
	m_variantFactory = new QtVariantEditorFactory(this);
	switch(editorType) {
	case GroupBox:
		m_propertyEditor = new QtGroupBoxPropertyBrowser;
		break;
	case Button:
		m_propertyEditor = new QtButtonPropertyBrowser;
		break;
	case Tree:
	default:
		m_propertyEditor = new QtTreePropertyBrowser;
		break;
	}
	m_propertyEditor->setFactoryForManager(m_variantManager, m_variantFactory);

	connect(m_variantManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
			SLOT(valueChanged(QtProperty*, const QVariant&)));

	generateUi(settings);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_propertyEditor);
	setLayout(layout);
}

//virtual
WSettingsEditor::~WSettingsEditor()
{
}

void WSettingsEditor::generateUi(QSettings& settings)
{
	{
		QMap<QtProperty*, QString>::iterator it = m_propertyToId.begin();
		QMap<QtProperty*, QString>::iterator itEnd = m_propertyToId.end();
		for (; it != itEnd; ++it) {
			delete it.key();
		}
	}
	m_propertyToId.clear();

	{
		QMap<QString, QtProperty*>::iterator it = m_idToGroup.begin();
		QMap<QString, QtProperty*>::iterator itEnd = m_idToGroup.end();
		for (; it != itEnd; ++it) {
			delete it.value();
		}
	}
	m_idToGroup.clear();

	m_idToProperty.clear();
	delete m_topProperty;

	// top property
	int groupId = QtVariantPropertyManager::groupTypeId();
	m_topProperty =
		m_variantManager->addProperty(groupId,m_title);

	// all other property
	m_normalizedKeyToOriginalKey.clear();
	m_arrayPropertyKeys.clear();
	QStringList keys = settings.allKeys();
	for (QStringList::iterator it = keys.begin(); it!=keys.end(); ++it) {
		QString& key = *it;
		QString arrayField;
		QString arrayIndex;
		QString parent;
		QString tmp;
		QString normalizedKey = key;
		do {
			split(key, tmp, arrayField);
			if (tmp.isEmpty()) {
				break;
			}

			split(tmp, parent, arrayIndex);
			if (parent.isEmpty()) {
				break;
			}

			bool isNumeric = false;
			int index = arrayIndex.toInt(&isNumeric);
			if (isNumeric) {
				normalizedKey = parent
								+ QLatin1Char('/')
								+ QString("%1").arg(index, 3, 10, QLatin1Char('0'))
								+ QLatin1Char('/')
								+ arrayField;
				m_arrayPropertyKeys.insert(parent);
			}
		} while (0);
		m_normalizedKeyToOriginalKey[normalizedKey] = key;
		key = normalizedKey;
	}
	keys.sort();

	QString parentKey, childKey;
	foreach (QString key, keys) {
		split(key, parentKey, childKey);

		QtProperty* parentProperty = addGroup(parentKey);
		QString actualKey = m_normalizedKeyToOriginalKey[key];

		if (childKey == QLatin1String("size")) {
			if (m_arrayPropertyKeys.contains(parentKey)) {
				// array size property should not be edited.
				continue;
			}
		}

		parentProperty->addSubProperty(addProperty(key, settings.value(actualKey)));
	}

	QtBrowserItem* editItem = m_propertyEditor->addProperty(m_topProperty);
	if (QtTreePropertyBrowser* treeEditor = qobject_cast<QtTreePropertyBrowser*>(m_propertyEditor)) {
		treeEditor->setRootIsDecorated(false);
		treeEditor->setExpanded(editItem, true);
		treeEditor->setAlternatingRowColors(true);
	}
}

void WSettingsEditor::valueChanged(QtProperty* property, const QVariant& value)
{
	if (m_propertyToId.contains(property)) {
		QString key = m_propertyToId[property];
		if (m_normalizedKeyToOriginalKey.contains(key)) {
			key = m_normalizedKeyToOriginalKey[key];
		}
		m_settings.setValue(key, value);
	}
}

QtVariantProperty* WSettingsEditor::addGroup(const QString& key)
{
	if (key.isEmpty()) {
		// no need to add(or ensure) group. just use top
		return m_topProperty;
	}

	if (m_idToGroup.contains(key)) {
		return m_idToGroup[key];
	}

	QString parentKey, myKey;
	split(key, parentKey, myKey);

	QString name = m_editorInfo.guiName(key);

	int groupTypeId = QtVariantPropertyManager::groupTypeId();
	QtVariantProperty* propertyGroup =
		m_variantManager->addProperty(groupTypeId, name);
	m_idToGroup[key] = propertyGroup;

	QtProperty* parentPropertyGroup = addGroup(parentKey);
	parentPropertyGroup->addSubProperty(propertyGroup);

	return propertyGroup;
}

QtVariantProperty* WSettingsEditor::addProperty(const QString& key,
												const QVariant& value)
{
	QString name = m_editorInfo.guiName(key);
	QString toolTip = m_editorInfo.guiToolTip(key);

	// preserve type as much as we can... :(
	QVariant typedCorrected = value;

	if (value.type() == QVariant::String) {
		QString valueInString = value.value<QString>();
		// we cannot simply use QVariant::canConvert<bool>()
		if (valueInString == QLatin1String("true") ||
			valueInString == QLatin1String("false")) {
			typedCorrected = value.value<bool>();
		}
	}

	QtVariantProperty* property = m_variantManager->addProperty(typedCorrected.type(), name);
	property->setValue(value);
	property->setToolTip(toolTip);
	property->setPropertyName(name);

	m_propertyToId[property] = key;
	m_idToProperty[key] = property;

	return property;
}

