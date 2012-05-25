#include "MainWindow.h"
#include <QSqlTableModel>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>

namespace {

enum {
	ColId = 0,
	ColName,
	ColMaxSpeed,
	ColMaxRange,
	ColWeight,
	ColDescription,

	ColCount,
};

}

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	model = new QSqlTableModel(this);
	model->setTable("scooter");
	model->setSort(ColName, Qt::AscendingOrder);
	model->setHeaderData(ColName, Qt::Horizontal, tr("Name"));
	model->setHeaderData(ColMaxSpeed, Qt::Horizontal, tr("MPH"));
	model->setHeaderData(ColMaxRange, Qt::Horizontal, tr("Miles"));
	model->setHeaderData(ColWeight, Qt::Horizontal, tr("Lbs"));
	model->setHeaderData(ColDescription, Qt::Horizontal, tr("Description"));
	model->select();

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
