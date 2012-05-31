#pragma once

#include <QSqlQueryModel>

class WSqlRelationalTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    WSqlRelationalTableModel(QObject* parent=0,
							 QSqlDatabase db = QSqlDatabase());
    virtual ~WSqlRelationalTableModel();
	virtual bool setTable(const QString& table);
signals:
public slots:
protected:
	// to get field index
	QSqlRecord m_fieldInfo;
};


{
	WSqlRelationTableDefintion rtd
		= WSqlRelationTableDefintion()
		.addColumn(employeeTable["name"])
		.addColumn(departmentTable["name"])
		.addColumn(locationTable["name"])
		.addColumn(employeeTable["age"]);

	QSqlQueryModel model;
	model->setQuery(rtd.query());
}
