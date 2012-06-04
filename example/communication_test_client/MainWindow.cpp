#include "MainWindow.h"
#include "WHistoryComboBox.h"
#include "WHistoryDateTimeEdit.h"
#include "WSimpleTableView.h"
#include "TripPlanner.h"
#include "TripPlanResultModel.h"
#include <QCoreApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QProgressBar>

MainWindow::MainWindow(QWidget* parent)
	: QDialog(parent)
{
	planner = new TripPlanner(this);

	QGroupBox* group = new QGroupBox(tr("Trip Information"));
	from = new WHistoryComboBox("from");
	to = new WHistoryComboBox("to");
	date = new WHistoryDateTimeEdit("date");
	date->setDisplayFormat(tr("yyyy-MM-dd"));
	time = new WHistoryDateTimeEdit("time");
	time->setDisplayFormat(tr("hh:mm::ss"));
	departure = new QRadioButton(tr("&Departure"));
	arrival = new QRadioButton(tr("&Arrival"));
	resultData = new TripPlanResultModel(this);
	result = new WSimpleTableView(resultData);

	// QStringList headerList = QStringList()
	// 						 << "Date"
	// 						 << "Departure"
	// 						 << "Arrival"
	// 						 << "Duration"
	// 						 << "Changes"
	// 						 << "Train type";
	// result->setColumnCount(headerList.size());
	// result->setHorizontalHeaderLabels( headerList );
	state = new QLabel(tr("Ready"));
	progressBar = new QProgressBar;
	progressBar->setRange(0,0);	// busy-indicator
	progressBar->hide();

	searchButton = new QPushButton(tr("&Search"));
	stopButton = new QPushButton(tr("&Stop"));
	stopButton->setEnabled(false);
	quitButton = new QPushButton(tr("&Quit"));

	connect(searchButton, SIGNAL(clicked()), SLOT(onBeginSearch()));
	connect(stopButton, SIGNAL(clicked()), SLOT(onStopSearch()));
	connect(stopButton, SIGNAL(clicked()), progressBar, SLOT(hide()));
	connect(quitButton, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(quit()));

	connect(planner, SIGNAL(stateChanged(const QString&)), state, SLOT(setText(const QString&)));
	connect(planner, SIGNAL(received(const TripPlanResultPacket&)),
			SLOT(addResult(const TripPlanResultPacket&)));
	connect(planner, SIGNAL(stoppedSearch()), SLOT(onStopSearch()));

	QHBoxLayout* radioLayout = new QHBoxLayout;
	radioLayout->addWidget(departure);
	radioLayout->addWidget(arrival);

	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow("&From:", from);
	formLayout->addRow("&To:", to);
	formLayout->addRow("&Date:", date);
	formLayout->addRow("T&ime:", time);
	formLayout->addRow(radioLayout);
	group->setLayout(formLayout);

	QHBoxLayout* progressLayout = new QHBoxLayout;
	progressLayout->addWidget(state);
	progressLayout->addWidget(progressBar);

	QVBoxLayout* leftLayout = new QVBoxLayout;
	leftLayout->addWidget(group);
	leftLayout->addWidget(result);
	leftLayout->addLayout(progressLayout);

	QVBoxLayout* rightLayout = new QVBoxLayout;
	rightLayout->addWidget(searchButton);
	rightLayout->addWidget(stopButton);
	rightLayout->addStretch(1);
	rightLayout->addWidget(quitButton);

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addLayout(leftLayout);
	mainLayout->addLayout(rightLayout);
	setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onBeginSearch()
{
	TripPlanQueryPacket query;
	query.from = from->currentText();
	query.to = to->currentText();
	query.time = QDateTime(date->date(),
						   time->time());
	query.isDeparture = departure->isChecked();

	resultData->clear();
	// result->setRowCount(0);
	progressBar->show();
	searchButton->setEnabled(false);
	stopButton->setEnabled(true);
	quitButton->setEnabled(false);
	planner->search(query);
}

void MainWindow::onStopSearch()
{
	progressBar->hide();
	searchButton->setEnabled(true);
	stopButton->setEnabled(false);
	quitButton->setEnabled(true);
}

void MainWindow::addResult(const TripPlanResultPacket& item)
{
	// QStringList headerList = QStringList()
	// 						 << "Date"
	// 						 << "Departure"
	// 						 << "Arrival"
	// 						 << "Duration"
	// 						 << "Changes"
	// 						 << "Train type";
	// QTime arrivalTime = item.departure.time().addSecs(item.duration);
	// QStringList fields;
	// fields << item.departure.date().toString(Qt::LocalDate)
	// 	   << item.departure.time().toString(tr("hh:mm"))
	// 	   << arrivalTime.toString(tr("hh:mm"))
	// 	   << tr("%1 hr %2 min").arg(item.duration / 60).arg(item.duration % 60)
	// 	   << QString::number(item.changes)
	// 	   << item.trainType;

	// int rowCount = result->rowCount();
	// result->setRowCount(rowCount+1);
	// int row = rowCount;
	// for (int i = 0; i < fields.count(); ++i) {
	// 	result->setItem(row, i, new QTableWidgetItem(fields[i]));
	// }
	resultData->addData(item);
}

