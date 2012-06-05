#pragma once

#include "common/logger/WLogOutput.h"
#include "gui/WMainWindow.h"
#include <QList>


class QCheckBox;
class QLineEdit;
class QPushButton;
class WHexEdit;
class WLogView;
class WSerialPort;
class WSerialPortPanel;

class MainWindow : public WMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
signals:
public slots:
	void openPort();
	void closePort();
	void updateUiState();
	void sendHexCommand(int index);
	void sendTextCommand(int index);
private slots:
	void onReadyRead();
protected:
	WLogOutputPtr m_logOutput;
	WSerialPort* m_serialPort;

	QPushButton* m_openButton;
	QPushButton* m_closeButton;
	WSerialPortPanel* m_serialPortPanel;
	WLogView* m_logView;
	QList<WHexEdit*> m_hexEditList;
	QList<QPushButton*> m_hexButtonList;
	QList<QLineEdit*> m_textEditList;
	QList<QPushButton*> m_textButtonList;
	QCheckBox* m_textLineFeedCheckBox;
};
