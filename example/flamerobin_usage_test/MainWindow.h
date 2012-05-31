#pragma once

#include "db/WSqlRelationalTableDefinition.h"
#include <QWidget>

class AutoFetchTableView;
class WSqlQueryModel;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
signals:
public slots:
private slots:
	void generateMore();
protected:
	AutoFetchTableView* m_fullTable;
	WSqlQueryModel* m_model;
};
