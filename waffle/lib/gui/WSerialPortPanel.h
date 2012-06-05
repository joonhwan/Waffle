#pragma once

#include "packet/WSerialPort.h"
#include <QWidget>

class QComboBox;
class QSpinBox;
class WEnumComboBox;
class WSerialPort;

class WSerialPortPanel : public QWidget
{
    Q_OBJECT
public:
    WSerialPortPanel(QWidget* parent=0);
    virtual ~WSerialPortPanel();
	void setData(const QString& portName,
				 BaudRateType baudRate = BAUD9600,
				 DataBitsType dataBits = DATA_8,
				 ParityType parity = PAR_NONE,
				 StopBitsType stopBits = STOP_1,
				 FlowType flow = FLOW_OFF);
	QString portName() const;
	BaudRateType baudRate() const;
	DataBitsType dataBits() const;
	ParityType parity() const;
	StopBitsType stopBits() const;
	FlowType flowControl() const;
	int timeoutMilisec() const;
signals:
	void configureChanged();
public slots:
	void setPortName(const QString& portName);
	void setBaudRate(BaudRateType baudrate);
	void setDataBits(DataBitsType databits);
	void setParity(ParityType parity);
	void setStopBits(StopBitsType stopbits);
    void setFlowControl(FlowType flowcontrol);
	void setTimeoutMiliSec(int value);
	void updateConfiguration(WSerialPort* serialPort);
private slots:
	void configureChangedSlot();
protected:
	WEnumComboBox* addCombo(const QMetaObject& obj, const char* enumName);
	QString settingGroupKey() const;
	void readSettings();
	void writeSettings();

	QComboBox* m_portCombo;
	WEnumComboBox* m_baudRateCombo;
	WEnumComboBox* m_dataBitsCombo;
	WEnumComboBox* m_stopBitsCombo;
	WEnumComboBox* m_parityCombo;
	WEnumComboBox* m_flowCombo;
	QSpinBox* m_timeoutMilisecSpin;
};
