#include "MainWindow.h"
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>

namespace {

enum {
	ColId = 0,
	ColName,
	ColDepartment,
	ColExtension,
	ColEmail,
	ColStartDate,

	ColCount,
};

}

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	QSqlRelationalTableModel* model = new QSqlRelationalTableModel(this);
	model->setTable("employee");
	model->setRelation(ColDepartment, QSqlRelation("department", "\"id\"", "\"name\""));
	// model->setSort(ColName, Qt::AscendingOrder);
	model->setHeaderData(ColName, Qt::Horizontal, tr("Name"));
	model->setHeaderData(ColDepartment, Qt::Horizontal, tr("Department"));
	model->setHeaderData(ColExtension, Qt::Horizontal, tr("Extension"));
	model->setHeaderData(ColEmail, Qt::Horizontal, tr("Email"));
	model->setHeaderData(ColStartDate, Qt::Horizontal, tr("StartDate"));
	model->select();

	// QModelIndex dItem = model->indexInQuery(model->index(0, 0));

	view = new QTableView;
	view->setModel(model);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setColumnHidden(ColId, true);
	view->resizeColumnsToContents();
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView* header = view->horizontalHeader();
	header->setStretchLastSection(true);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(view);
	setLayout(layout);
}

//virtual
MainWindow::~MainWindow()
{
}
