#pragma once

#include <QMainWindow>

class WMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    WMainWindow(QWidget* parent=0);
    virtual ~WMainWindow();
protected:
	virtual void showEvent(QShowEvent* e);
	virtual void closeEvent(QCloseEvent* e);
	void restoreWindowState();
	QString settingName() const;
private:
	bool m_showedOnce;
};
