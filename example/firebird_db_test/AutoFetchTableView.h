#pragma once

#include <QTableView>

class QTimer;

class AutoFetchTableView : public QTableView
{
    Q_OBJECT
public:
    AutoFetchTableView(QWidget* parent=0);
    virtual ~AutoFetchTableView();
signals:
public slots:
	virtual void reset();
private slots:
	void tryFetchMore();
protected:
	QTimer* m_timer;
};
