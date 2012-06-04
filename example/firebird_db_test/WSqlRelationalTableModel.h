#pragma once

#include <QSqlQueryModel>

class WSqlRelationalTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    WSqlRelationalQueryModel(QObject* parent=0);
    virtual ~WSqlRelationalQueryModel();
	WSqlRelationalTableModel& addColumn(
signals:
public slots:
protected:
};
