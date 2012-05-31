#pragma once

#include <QSqlQueryModel>

class WSqlRelationalTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    WSqlRelationalQueryModel(QObject* parent=0);
    virtual ~WSqlRelationalQueryModel();
	WSqlRelationalTableModel& addColumn(WSqlFieldDefinition& fd
signals:
public slots:
protected:
};
