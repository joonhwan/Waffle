#include "WSerialPortPanel.h"
#include "gui/WEnumComboBox.h"
#include "packet/WSerialPortEnumerator.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QSettings>

WSerialPortPanel::WSerialPortPanel(QWidget* parent)
{
	m_portCombo = new QComboBox;

	// QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
	// foreach(const QextPortInfo& port, ports) {
	// 	if (!port.portName.isEmpty()) {
	// 		m_portCombo->addItem(QString::fromLatin1("%1").arg(port.portName),
	// 							 QVariant::fromValue<QextPortInfo>(port));
	// 	}
	// }
	QStringList ports = WSerialPortEnumerator::getPorts();
	foreach(const QString& port, ports) {
		if (!port.isEmpty()) {
			m_portCombo->addItem(port);
		}
	}

	const QMetaObject& mo = QextSerialPortEnums::staticMetaObject;
	m_baudRateCombo = addCombo(mo, "BaudRateType");
	m_dataBitsCombo = addCombo(mo, "DataBitsType");
	m_stopBitsCombo = addCombo(mo, "StopBitsType");
	m_parityCombo = addCombo(mo, "ParityType");
	m_flowCombo = addCombo(mo, "FlowType");
	m_timeoutMilisecSpin = new QSpinBox;
	m_timeoutMilisecSpin->setRange(0, 6000000);
	m_timeoutMilisecSpin->setPrefix(tr(" msec"));
	connect(m_timeoutMilisecSpin, SIGNAL(valueChanged(int)),
			this, SLOT(configureChanged()));

	QFormLayout* layout = new QFormLayout;
	layout->addRow(tr("&Port:"), m_portCombo);
	layout->addRow(tr("&Baudrate:"), m_baudRateCombo);
	layout->addRow(tr("&Stopbits:"), m_stopBitsCombo);
	layout->addRow(tr("Pa&rity:"), m_parityCombo);
	layout->addRow(tr("&Flow:"), m_flowCombo);
	setLayout(layout);

	readSettings();
}

//virtual
WSerialPortPanel::~WSerialPortPanel()
{
	// writeSettings();
}

void WSerialPortPanel::setData(const QString& portName,
							   BaudRateType baudRate,
							   DataBitsType dataBits,
							   ParityType parity,
							   StopBitsType stopBits,
							   FlowType flow)
{
	setPortName(portName);
	setBaudRate(baudRate);
	setParity(parity);
	setStopBits(stopBits);
	setFlowControl(flow);
}


QString WSerialPortPanel::portName() const
{
	return m_portCombo->currentText();
}

void WSerialPortPanel::setPortName(const QString& portName)
{
	int index = m_portCombo->findText(portName);
	if (index>=0) {
		m_portCombo->setCurrentIndex(index);
	}
}

BaudRateType WSerialPortPanel::baudRate() const
{
	return (BaudRateType)m_baudRateCombo->currentEnumInt();
}

void WSerialPortPanel::setBaudRate(BaudRateType baudrate)
{
	m_baudRateCombo->setCurrentEnumInt((int)baudrate);
}

DataBitsType WSerialPortPanel::dataBits() const
{
	return (DataBitsType)m_dataBitsCombo->currentEnumInt();
}

void WSerialPortPanel::setDataBits(DataBitsType databits)
{
	m_dataBitsCombo->setCurrentEnumInt((int)databits);
}

ParityType WSerialPortPanel::parity() const
{
	return (ParityType)m_parityCombo->currentEnumInt();
}

void WSerialPortPanel::setParity(ParityType parity)
{
	m_parityCombo->setCurrentEnumInt((int)parity);
}

StopBitsType WSerialPortPanel::stopBits() const
{
	return (StopBitsType)m_stopBitsCombo->currentEnumInt();
}

void WSerialPortPanel::setStopBits(StopBitsType stopbits)
{
	m_stopBitsCombo->setCurrentEnumInt((int)stopbits);
}

FlowType WSerialPortPanel::flowControl() const
{
	return (FlowType)m_flowCombo->currentEnumInt();
}

void WSerialPortPanel::setFlowControl(FlowType flowcontrol)
{
	m_flowCombo->setCurrentEnumInt((int)flowcontrol);
}

int WSerialPortPanel::timeoutMilisec() const
{
	return m_timeoutMilisecSpin->value();
}

void WSerialPortPanel::setTimeoutMiliSec(int value)
{
	m_timeoutMilisecSpin->setValue(value);
}

void WSerialPortPanel::configureChangedSlot()
{
	writeSettings();
	emit configureChanged();
}

WEnumComboBox* WSerialPortPanel::addCombo(const QMetaObject& mo, const char* enumName)
{
	WEnumComboBox* combo = new WEnumComboBox(mo, enumName);
	combo->setEditable(false);
	connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(configureChangedSlot()));
	return combo;
}

QString WSerialPortPanel::settingGroupKey() const
{
	QString panelName;
	if (objectName().isEmpty()) {
		panelName = QString::fromLatin1("WSerialPortPanel");
	} else {
		panelName = objectName();
	}
	return panelName;
}

void WSerialPortPanel::readSettings()
{
	QSettings s;
	s.beginGroup(settingGroupKey());
	setPortName(s.value("portName").toString());
	setBaudRate((BaudRateType)s.value("baudrate").toInt());
	setDataBits((DataBitsType)s.value("databits").toInt());
	setParity((ParityType)s.value("parity").toInt());
	setFlowControl((FlowType)s.value("flowcontrol").toInt());
	setTimeoutMiliSec(s.value("timeout").toInt());
	s.endGroup();
}

void WSerialPortPanel::writeSettings()
{
	QSettings s;
	s.beginGroup(settingGroupKey());
	s.setValue("portName", portName());
	s.setValue("baudrate", (int)baudRate());
	s.setValue("databits", (int)dataBits());
	s.setValue("parity", (int)parity());
	s.setValue("flowcontrol", (int)flowControl());
	s.setValue("timeout", m_timeoutMilisecSpin->value());
	s.endGroup();
}

