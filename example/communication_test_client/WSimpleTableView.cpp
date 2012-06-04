#include "WSimpleTableView.h"
#include "WSimpleTableModel.h"
#include <QApplication>
#include <QAction>
#include <QClipboard>

WSimpleTableView::WSimpleTableView(QAbstractItemModel* model, QWidget* parent)
	: QTableView(parent)
{
	copyAction = new QAction(tr("&Copy"), this);
	copyAction->setShortcut(QKeySequence::Copy);
	this->addAction(copyAction);

	connect(copyAction, SIGNAL(triggered()), SLOT(copy()));

	setModel(model);
	if (WSimpleTableModel* stm = qobject_cast<WSimpleTableModel*>(model)) {
		if (stm->vertical()) {
			setSelectionBehavior(QAbstractItemView::SelectRows);
		} else {
			setSelectionBehavior(QAbstractItemView::SelectColumns);
		}
	}
}

//virtual
WSimpleTableView::~WSimpleTableView()
{
}

void WSimpleTableView::copy()
{
	QItemSelectionModel* selectModel = this->selectionModel();
	QAbstractItemModel* model = this->model();
	bool rowWise = true;
	if (WSimpleTableModel* stm = qobject_cast<WSimpleTableModel*>(this->model())) {
		if (!stm->vertical()) {
			rowWise = false;
		}
	}
	QLatin1String tab("\t");
	QLatin1String lineFeed("\n");
	if (selectModel->hasSelection()) {
		QModelIndexList selected = selectModel->selectedIndexes();
		qSort(selected);

		QString copied;
		int lastRow = -1;
		foreach(const QModelIndex& index, selected) {
			if (lastRow!=index.row()) {
				if (lastRow>=0) {
					copied += lineFeed;
				}
			} else {
				copied += tab;
			}
			copied += model->data(index, Qt::DisplayRole).toString();

			lastRow = index.row();
		}

		QApplication::clipboard()->setText(copied);
	}
}
