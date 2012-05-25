#pragma once

#include <QWidget>

class QSqlRelationalTableModel;
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
	// QSqlRelationalTableModel* model;
	QTableView* view;
};
