#pragma once

#include <QMainWindow>

class WSerialPortPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
signals:
public slots:
protected:
	WSerialPortPanel* m_serialPortPanel;
};
