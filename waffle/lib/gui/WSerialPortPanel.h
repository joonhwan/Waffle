#pragma once

#include "packet/WSerialPort.h"
#include <QWidget>

class QComboBox;
class QSpinBox;
class WEnumComboBox;

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
	void setPortName(const QString& portName);
	BaudRateType baudRate() const;
	void setBaudRate(BaudRateType baudrate);
	DataBitsType dataBits() const;
	void setDataBits(DataBitsType databits);
	ParityType parity() const;
	void setParity(ParityType parity);
	StopBitsType stopBits() const;
	void setStopBits(StopBitsType stopbits);
    FlowType flowControl() const;
	void setFlowControl(FlowType flowcontrol);
	int timeoutMilisec() const;
	void setTimeoutMiliSec(int value);
signals:
	void configureChanged();
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
