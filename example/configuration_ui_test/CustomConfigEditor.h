#pragma once

#include <QWidget>
#include "gui/WSettingExchangeableEditor.h"

class AppConfig;
class QSettings;
class QPushButton;
class WSettingExchangeableEditor;

class WSettingCustomEditorImpl : public QWidget
{
    Q_OBJECT
public:
    WSettingCustomEditorImpl(QWidget* parent=0);
    virtual ~WSettingCustomEditorImpl();
	void setSaveButton(QPushButton* saveButton);
	void setRestoreButton(QPushButton* restoreButton);
signals:
private slots:
	void savePersistantly();
protected:
	WSettingExchangeableEditor* m_inEditor;
	QPushButton* m_saveButton;
	bool m_saveButtonCustomized;
	QPushButton* m_restoreButton;
	bool m_restoreButtonCustomized;
	virtual void save() = 0;
	void generateUi(const QString& title, WSettingExchangeable& config,
					WSettingExchangeableEditor::Type type);
};

template<typename T>
class WSettingCustomConfigEditor : public WSettingCustomEditorImpl
{
public:
	WSettingCustomConfigEditor(const QString& rootKey,
					   QSettings& settings,
					   WSettingExchangeableEditor::Type type = WSettingExchangeableEditor::Button,
					   QWidget* parent = 0)
		: WSettingCustomEditorImpl(parent)
		, m_settings(settings)
		, m_rootKey(rootKey)
	{
		m_config.loadFrom(m_settings, rootKey);

		generateUi(rootKey, m_config, type);
	}
	virtual void save()
	{
		m_config.saveTo(m_settings, m_rootKey);
	}
	T m_config;
	QString m_rootKey;
	QSettings& m_settings;
};
