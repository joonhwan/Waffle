#include "MainWindow.h"
#include "AutoFetchTableView.h"
#include "WSqlQueryModel.h"
#include "Tables.h"
#include "db/WSqlQueryBuilder.h"
#include "common/global/WRandom.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QDate>

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	m_model = new WSqlQueryModel(this);

	m_fullTable = new AutoFetchTableView;
	m_fullTable->setModel(m_model);

	QPushButton* generateButton = new QPushButton(tr("Generate more random data"));
	connect(generateButton, SIGNAL(clicked()), SLOT(generateMore()));

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(m_fullTable);
	layout->addWidget(generateButton);
	setLayout(layout);

	generateMore();
}

//virtual
MainWindow::~MainWindow()
{
}

void MainWindow::generateMore()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.transaction();
	WRandom randomNumber(1950, 2012);
	for (int i = 0; i < 10; ++i) {
		QSqlQuery query =
			WSqlSimpleInsertQueryBuilder("location")
			.keyValue("name", QString("area%1").arg(i))
			.query();
		if (!query.isActive()) {
			qDebug() << query.lastError().text();
		}
	}
	db.commit();

	WRandom randomLocation(1, 8);

	db.transaction();
	for (int i = 0; i < 100; ++i) {
		QSqlQuery query =
			WSqlSimpleInsertQueryBuilder("department")
			.keyValue("name", QString("dept%1").arg(i))
			.keyValue("code", QString("D%1").arg(i))
			.keyValue("location_id", randomLocation.generate())
			.query();
		if (!query.isActive()) {
			qDebug() << query.lastError().text();
		}
	}
	db.commit();

	WRandom randomDept(1, 99);
	WRandom randomAge(19, 60);
	db.transaction();
	for (int i = 0; i < 1000; ++i) {
		QSqlQuery query =
			WSqlSimpleInsertQueryBuilder("employee")
			.keyValue("name", QString("employe%1").arg(i))
			.keyValue("department_id", randomDept.generate())
			.keyValue("extension", randomNumber.generate())
			.keyValue("email", QString("acount_%1@yahoo.com").arg(i))
			.keyValue("startdate", QDate::currentDate())
			.keyValue("age", randomAge.generate())
			.query();
		if (!query.isActive()) {
			qDebug() << query.lastError().text();
		}
	}
	db.commit();

	m_model->setDefinition(allEmployeeData);
}
