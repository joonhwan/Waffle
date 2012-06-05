#include "MainWindow.h"
#include "common/logger/WLogger.h"
#include "common/logger/wlogguioutput.h"
#include "gui/WHexEdit.h"
#include "gui/WLogView.h"
#include "gui/WSerialPortPanel.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {

const int HEX_EDITOR_COUNT = 4;
const int TEXT_EDITOR_COUNT = 4;
const char* logger = "SERIALTEST";

};

MainWindow::MainWindow(QWidget* parent)
	: WMainWindow(parent)
{
	m_serialPort = new WSerialPort(WSerialPort::EventDriven, this);
	connect(m_serialPort, SIGNAL(readyRead()), SLOT(onReadyRead()));

	m_serialPortPanel = new WSerialPortPanel(this);
	m_logView = new WLogView;

	m_logOutput = WLogOutputPtr(new WLogGuiOutput);
	WLogger::instance().addLogOutput(m_logOutput);
	m_logOutput->connect(m_logView, SLOT(addLog(const WLogEvent&)));

	m_openButton = new QPushButton(tr("&Open"));
	m_closeButton = new QPushButton(tr("&Close"));

	for (int i = 0; i < HEX_EDITOR_COUNT; ++i) {
		WHexEdit* hexEditor = new WHexEdit;
		hexEditor->setOverwriteMode(false);
		hexEditor->setAddressArea(false);
		m_hexEditList << hexEditor;

		QPushButton* hexButton = new QPushButton(tr("Send B&%1").arg(i+1));
		m_hexButtonList << hexButton;
	}

	for (int i = 0; i < TEXT_EDITOR_COUNT; ++i) {
		QLineEdit* textEdit = new QLineEdit;
		m_textEditList << textEdit;

		QPushButton* textButton = new QPushButton(tr("Send T&%1").arg(i+1));
		m_textButtonList << textButton;
	}
	m_textLineFeedCheckBox = new QCheckBox(tr("CR/LF"));

	//

	connect(m_openButton, SIGNAL(clicked()), SLOT(openPort()));
	connect(m_closeButton, SIGNAL(clicked()), SLOT(closePort()));

	QSignalMapper* hexButtonMapper = new QSignalMapper(this);
	for (int i = 0; i < HEX_EDITOR_COUNT; ++i) {
		QPushButton* button = m_hexButtonList.at(i);
		hexButtonMapper->setMapping(button, i);
		connect(button, SIGNAL(clicked()),
				hexButtonMapper, SLOT(map()));
	}
	connect(hexButtonMapper, SIGNAL(mapped(int)), SLOT(sendHexCommand(int)));

	QSignalMapper* textButtonMapper = new QSignalMapper(this);
	for (int i = 0; i < TEXT_EDITOR_COUNT; ++i) {
		QPushButton* button = m_textButtonList.at(i);
		textButtonMapper->setMapping(button, i);
		connect(button, SIGNAL(clicked()),
				textButtonMapper, SLOT(map()));
	}
	connect(textButtonMapper, SIGNAL(mapped(int)), SLOT(sendTextCommand(int)));


	//

	QHBoxLayout* serialPanelButtonLayout = new QHBoxLayout;
	serialPanelButtonLayout->addWidget(m_openButton);
	serialPanelButtonLayout->addWidget(m_closeButton);

	QGroupBox* serialPanel = new QGroupBox(tr("&Connection"));
	QVBoxLayout* serialPanelLayout = new QVBoxLayout;
	serialPanelLayout->addWidget(m_serialPortPanel);
	serialPanelLayout->addLayout(serialPanelButtonLayout);
	serialPanel->setLayout(serialPanelLayout);

	QGroupBox* hexEditPanel = new QGroupBox(tr("&Transmit"));
	QGridLayout* hexLayout = new QGridLayout;
	for (int i = 0; i < HEX_EDITOR_COUNT; ++i) {
		QLabel* hexLabel = new QLabel(tr("B&%1").arg(i+1));
		hexLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		hexLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

		WHexEdit* hexEdit = m_hexEditList.at(i);
		hexLabel->setBuddy(hexEdit);

		QPushButton* hexButton = m_hexButtonList.at(i);
		hexButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

		hexLayout->addWidget(hexLabel, i, 0);
		hexLayout->addWidget(hexEdit, i, 1);
		hexLayout->addWidget(hexButton, i, 2);
	}
	hexLayout->setColumnStretch(1, 1);
	hexEditPanel->setLayout(hexLayout);

	QGroupBox* textEditPanel = new QGroupBox(tr("&Transmit"));
	QGridLayout* textLayout = new QGridLayout;
	for (int i = 0; i < TEXT_EDITOR_COUNT; ++i) {
		QLabel* textLabel = new QLabel(tr("T&%1").arg(i+1));
		textLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		textLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

		QLineEdit* textEdit = m_textEditList.at(i);
		textLabel->setBuddy(textEdit);

		QPushButton* textButton = m_textButtonList.at(i);
		textButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

		textLayout->addWidget(textLabel, i, 0);
		textLayout->addWidget(textEdit, i, 1);
		textLayout->addWidget(textButton, i, 2);
	}
	textLayout->setColumnStretch(1, 1);
	textLayout->addWidget(m_textLineFeedCheckBox, TEXT_EDITOR_COUNT, 0, 1, -1);
	textEditPanel->setLayout(textLayout);

	QTabWidget* tabWidget = new QTabWidget;
	tabWidget->addTab(hexEditPanel, tr("Transmit &Binary"));
	tabWidget->addTab(textEditPanel, tr("Transmit &Text"));

	QWidget* mainWidget = new QWidget;
	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addWidget(serialPanel);
	mainLayout->addWidget(tabWidget);
	mainWidget->setLayout(mainLayout);

	QGroupBox* logViewGroup = new QGroupBox(tr("L&og"));
	QVBoxLayout* logViewGroupLayout = new QVBoxLayout;
	logViewGroupLayout->addWidget(m_logView);
	logViewGroup->setLayout(logViewGroupLayout);

	QSplitter* central = new QSplitter(Qt::Vertical);
	central->addWidget(mainWidget);
	central->addWidget(logViewGroup);

	setCentralWidget(central);

	updateUiState();
	WLOG_INFO(logger) << "ui generated.";
}

//virtual
MainWindow::~MainWindow()
{
}

void MainWindow::openPort()
{
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
	}
	m_serialPortPanel->updateConfiguration(m_serialPort);
	m_serialPort->open(QIODevice::ReadWrite);
	if (m_serialPort->isOpen()) {
		WLOG_INFO(logger) << "opened " << m_serialPort->portName() << " successfly";
	} else {
		WLOG_WARN(logger) << "unable to open port!";
	}

	updateUiState();
}

void MainWindow::closePort()
{
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
	}

	if (m_serialPort->isOpen()) {
		WLOG_INFO(logger) << "unable to close " << m_serialPort->portName();
	} else {
		WLOG_INFO(logger) << "closed " << m_serialPort->portName() << ".";
	}

	updateUiState();
}

void MainWindow::updateUiState()
{
	bool opened = m_serialPort->isOpen();
	m_openButton->setEnabled(!opened);
	m_closeButton->setEnabled(opened);

	for (int i = 0; i < HEX_EDITOR_COUNT; ++i) {
		m_hexButtonList.at(i)->setEnabled(opened);
	}
	for (int i = 0; i < TEXT_EDITOR_COUNT; ++i) {
		m_textButtonList.at(i)->setEnabled(opened);
	}
}

void MainWindow::sendHexCommand(int index)
{
	if (index < HEX_EDITOR_COUNT) {
		QByteArray ba = m_hexEditList.at(index)->data();
		if (!ba.isEmpty()) {
			WLOG_INFO(logger) << "sending [" << ba << "] ....";
			qint64 sentBytes = m_serialPort->write(ba);
			if ((int)sentBytes == ba.size()) {
				WLOG_INFO(logger) << "sent successfully...";
			} else {
				WLOG_WARN(logger) << "unable to send!";
			}
		}
	}
}

void MainWindow::sendTextCommand(int index)
{
	if (index < TEXT_EDITOR_COUNT) {
		QByteArray ba = m_textEditList.at(index)->text().toAscii();
		bool lineFeed = m_textLineFeedCheckBox->isChecked();
		if (!ba.isEmpty()) {
			QByteArray escaped(ba.length(), 0);
			qsnprintf(escaped.data(), escaped.length(),
					  ba.constData());
			WLOG_INFO(logger) << "sending [" << ba << "] "
							  << (lineFeed ? "with Line Feed" : "")
							  << ".";
			qint64 sentBytes = m_serialPort->write(ba);
			if ((int)sentBytes == ba.size()) {
				WLOG_INFO(logger) << "sent successfully...";
			} else {
				WLOG_WARN(logger) << "unable to send!";
			}
		}
	}
}

void MainWindow::onReadyRead()
{
	QByteArray ba;
	int bytes = m_serialPort->bytesAvailable();
	if (bytes > 0) {
		ba = m_serialPort->read(bytes);
	}

	WLOG_INFO(logger) << "received [" << ba << "].";
}
