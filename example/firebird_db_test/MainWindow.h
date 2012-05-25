#pragma once

#include <QWidget>

class QSqlTableModel;
class QTableView;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
signals:
public slots:
protected:
	QSqlTableModel* model;
	QTableView* view;
};
