#include "AutoFetchTableView.h"
#include <QTimer>
#include <QDebug>

AutoFetchTableView::AutoFetchTableView(QWidget* parent)
	: QTableView(parent)
{
}

//virtual
AutoFetchTableView::~AutoFetchTableView()
{
}

//virtual
void AutoFetchTableView::reset()
{
	QTableView::reset();
}

void AutoFetchTableView::tryFetchMore()
{
	if (model()->canFetchMore(QModelIndex())) {
		model()->fetchMore(QModelIndex());
		qDebug() << "fetched more... count: " << model()->rowCount();
	} else {
		m_timer->stop();
		qDebug() << "fetch more stopped.";
	}
}
