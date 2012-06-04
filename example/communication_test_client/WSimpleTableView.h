#pragma once

#include <QTableView>

class QAction;

class WSimpleTableView : public QTableView
{
    Q_OBJECT
public:
    WSimpleTableView(QAbstractItemModel* model, QWidget* parent=0);
    virtual ~WSimpleTableView();
signals:
public slots:
	void copy();
protected:
private:
	QAction *copyAction;
};
