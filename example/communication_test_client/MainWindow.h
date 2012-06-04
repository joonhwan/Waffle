#pragma once

#include <QDialog>
#include "Protocol.h"

class QDateTimeEdit;
class QLabel;
class QProgressBar;
class QRadioButton;
class QTableView;
class TripPlanner;
class TripPlanResultModel;
class WHistoryComboBox;

class MainWindow : public QDialog
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
private slots:
	void onBeginSearch();
	void onStopSearch();
	void addResult(const TripPlanResultPacket& resultItem);
private:
	TripPlanner* planner;
	TripPlanResultModel* resultData;
	WHistoryComboBox* from;
	WHistoryComboBox* to;
	QDateTimeEdit* date;
	QDateTimeEdit* time;
	QRadioButton* departure;
	QRadioButton* arrival;
	QTableView* result;
	QPushButton* searchButton;
	QPushButton* stopButton;
	QPushButton* quitButton;
	QLabel* state;
	QProgressBar* progressBar;
};
