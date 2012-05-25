#include "WSettingExchangeableEditor.h"
#include "common/config/WSettingExchangeable.h"
#include "common/config/WSettingExchanger.h"
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

WSettingExchangeableEditor::WSettingExchangeableEditor(WSettingExchangeable& exchangeable,
													   const QString& title,
													   Type editorType,
													   QWidget* parent)
	: QWidget(parent)
	, WSettingAccessor(WSettingAccessor::Write) // write configuration structure/value into editor
	, m_exchangeable(exchangeable)
	, m_editorType(editorType)
	, m_title(title)
	, m_topProperty(0)
	, m_dirty(false)
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

	generateUi(exchangeable);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_propertyEditor);
	setLayout(layout);
}

//virtual
WSettingExchangeableEditor::~WSettingExchangeableEditor()
{
}

//virtual
bool WSettingExchangeableEditor::hasKeyStartsWith(const QString& _key)
{
	QString key = m_currentBasePath.join("/") + "/" + _key;
	return m_idToProperty.contains(key);
}

//virtual
void WSettingExchangeableEditor::beginGroup(const QString& _key)
{
	m_currentBasePath.push_back(_key);
	QString basePath = m_currentBasePath.join("/");
	QtVariantProperty* group = addGroup(basePath);
	if (m_currentParentProperty.size()) {
		m_currentParentProperty.last()->addSubProperty(group);
	}
	m_currentParentProperty.push_back(group);
}

//virtual
void WSettingExchangeableEditor::endGroup()
{
	m_currentBasePath.pop_back();
	m_currentParentProperty.pop_back();
}

//virtual
int WSettingExchangeableEditor::beginArray(const QString& _key)
{
	// same impl in editor's case
	beginGroup(_key);
	return 0; // TODO check
}

//virtual
void WSettingExchangeableEditor::beginArrayIndex(int index)
{
	QString indexString = QString("%1").arg(index);
	m_currentBasePath.push_back(indexString);
	QtVariantProperty* parent = addGroup(indexString);
	if (m_currentParentProperty.size()) {
		m_currentParentProperty.last()->addSubProperty(parent);
	}
	m_currentParentProperty.push_back(parent);
}

//virtual
void WSettingExchangeableEditor::endArrayIndex()
{
	m_currentBasePath.pop_back();
	m_currentParentProperty.pop_back();
}

//virtual
void WSettingExchangeableEditor::endArray()
{
	// same impl in editor's case
	endGroup();
}

//virtual
QVariant WSettingExchangeableEditor::value(const QString& _key, const QVariant& defaultValue)
{
	QString key = m_currentBasePath.join("/") + "/" + _key;
	QVariant v = defaultValue;
	if (m_idToProperty.contains(key)) {
		QtVariantProperty* property = m_idToProperty[key];
		v= property->value();
	}
	return v;
}

//virtual
void WSettingExchangeableEditor::setValue(const QString& _key, const QVariant& value)
{
	QString key = m_currentBasePath.join("/") + "/" + _key;
	if (m_idToProperty.contains(key)) {
		QtVariantProperty* property = m_idToProperty[key];
		int propertyType = property->propertyType();
		if (propertyType == QtVariantPropertyManager::enumTypeId()) {
			QStringList enumNames = property->attributeValue("enumNames").toStringList();
			if (value.type()==QVariant::String) {
				int index = enumNames.indexOf(value.toString());
				Q_ASSERT(index != -1);
				if (index != -1) {
					property->setValue(index);
				} else {
					property->setValue(0);
				}
			} else {
				property->setValue(value);
			}
		}
	} else {
		m_currentParentProperty.last()->addSubProperty(addProperty(key, value));
	}
}

//virtual
void WSettingExchangeableEditor::setEnumListHint(const QString& _key,
												 const QStringList& enumList)
{
	QString key = m_currentBasePath.join("/") + "/" + _key;
	if (!m_idToProperty.contains(key)) {
		QtVariantProperty* enumProperty = addEnum(key, enumList);
		m_currentParentProperty.last()->addSubProperty(enumProperty);
	}
}

void WSettingExchangeableEditor::save()
{
	WSettingExchanger e(*this);
	this->m_type = WSettingAccessor::Read;
	m_exchangeable.exchange(e);
	this->m_type = WSettingAccessor::Write;

	m_dirty = false;
	emit saved();
}

void WSettingExchangeableEditor::restore()
{
	WSettingExchanger e(*this);
	this->m_type = WSettingAccessor::Write;
	m_exchangeable.exchange(e);

	m_dirty = false;
	emit restored();
}

void WSettingExchangeableEditor::generateUi(WSettingExchangeable& exchangeable)
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

	m_currentParentProperty.clear();
	m_currentBasePath.clear();

	m_currentBasePath.push_back(m_title);

	// top property
	int groupId = QtVariantPropertyManager::groupTypeId();
	m_topProperty =
		m_variantManager->addProperty(groupId,m_title);

	m_currentParentProperty.push_back(m_topProperty);

	// all other property
	QString parentKey, childKey;
	WSettingExchanger e(*this);
	exchangeable.exchange(e);

	QtBrowserItem* editItem = m_propertyEditor->addProperty(m_topProperty);
	if (QtTreePropertyBrowser* treeEditor = qobject_cast<QtTreePropertyBrowser*>(m_propertyEditor)) {
		treeEditor->setRootIsDecorated(false);
		treeEditor->setExpanded(editItem, true);
		treeEditor->setAlternatingRowColors(true);
	}
}

void WSettingExchangeableEditor::valueChanged(QtProperty* property, const QVariant& value)
{
	if (m_propertyToId.contains(property)) {
		QString key = m_propertyToId[property];
		if (m_normalizedKeyToOriginalKey.contains(key)) {
			key = m_normalizedKeyToOriginalKey[key];
		}
		// m_exchangeable.setValue(key, value);
		if (!m_dirty) {
			m_dirty = true;
			emit modified();
		}
	}
}

QtVariantProperty* WSettingExchangeableEditor::addGroup(const QString& key)
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

	return propertyGroup;
}

QtVariantProperty* WSettingExchangeableEditor::addEnum(const QString& key,
													   const QStringList& enumNames)
{
	QString name = m_editorInfo.guiName(key);
	QString toolTip = m_editorInfo.guiToolTip(key);

	int enumTypeId = QtVariantPropertyManager::enumTypeId();
	QtVariantProperty* property = m_variantManager->addProperty(enumTypeId, name);
	property->setToolTip(toolTip);
	property->setPropertyName(name);
	property->setAttribute("enumNames", enumNames);

	m_propertyToId[property] = key;
	m_idToProperty[key] = property;

	return property;
}

QtVariantProperty* WSettingExchangeableEditor::addProperty(const QString& key,
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

