// -*- coding:utf-8 -*-

#include "CustomConfigEditor.h"
#include "AppConfig.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>

WSettingCustomEditorImpl::WSettingCustomEditorImpl(QWidget* parent)
	: QWidget(parent)
	, m_saveButtonCustomized(false)
	, m_restoreButtonCustomized(false)
{
}

//virtual
WSettingCustomEditorImpl::~WSettingCustomEditorImpl()
{
}

void WSettingCustomEditorImpl::setSaveButton(QPushButton* saveButton)
{
	if (!m_saveButtonCustomized && m_saveButton) {
		m_saveButton->hide();
		disconnect(m_inEditor, 0, m_saveButton, 0);
		m_saveButtonCustomized = true;
	}
	m_saveButton = saveButton;
	connect(m_saveButton, SIGNAL(clicked()),
			m_inEditor, SLOT(save()));
}

void WSettingCustomEditorImpl::setRestoreButton(QPushButton* restoreButton)
{
	if (!m_restoreButtonCustomized && m_restoreButton) {
		m_restoreButton->hide();
		disconnect(m_inEditor, 0, m_restoreButton, 0);
		m_restoreButtonCustomized = true;
	}
	m_restoreButton = restoreButton;
	connect(m_restoreButton, SIGNAL(clicked()),
			m_inEditor, SLOT(save()));
}

void WSettingCustomEditorImpl::generateUi(const QString& title, WSettingExchangeable& config,
									WSettingExchangeableEditor::Type type)
{
	QVBoxLayout* layout = new QVBoxLayout;
	m_inEditor
		= new WSettingExchangeableEditor(config, title, type);
	connect(m_inEditor, SIGNAL(saved()), SLOT(savePersistantly()));

	m_saveButton = new QPushButton("&Save");
	m_restoreButton = new QPushButton("&Restore");
	connect(m_saveButton, SIGNAL(clicked()),
			m_inEditor, SLOT(save()));
	connect(m_restoreButton, SIGNAL(clicked()),
			m_inEditor, SLOT(restore()));
	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(m_saveButton);
	buttonLayout->addWidget(m_restoreButton);
	layout->addWidget(m_inEditor);
	layout->addLayout(buttonLayout);

	this->setLayout(layout);
}

void WSettingCustomEditorImpl::savePersistantly()
{
	// appConfig->saveTo(m_settings, "AppConfig");
	save();
}
