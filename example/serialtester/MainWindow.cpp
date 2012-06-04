#include "MainWindow.h"
#include "gui/WSerialPortPanel.h"

MainWindow::MainWindow(QWidget* parent)
{
	m_serialPortPanel = new WSerialPortPanel(this);
	setCentralWidget(m_serialPortPanel);
}

//virtual
MainWindow::~MainWindow()
{
}
