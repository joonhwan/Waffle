#include "WHistoryComboBox.h"
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
#include <QSortFilterProxyModel>

namespace {

const char* COMBO_HISTORY_GROUP = "WHistoryComboBox";

}

WHistoryComboBox::WHistoryComboBox(const QString& comboBoxName, QWidget* parent)
	: QComboBox(parent)
	, m_comboBoxName(comboBoxName)
	, m_restoring(false)
{
	setEditable(true); // by default
	view()->viewport()->installEventFilter(this);
	// QAbstractItemModel* comboModel = model();
	// QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
	// proxyModel->setSourceModel(comboModel);
	// setModel(proxyModel);

	connect(this, SIGNAL(currentIndexChanged(int)),
			SLOT(registerCurrentItem()));
	// connect(this, SIGNAL(editTextChanged(const QString&)),
	// 		SLOT(registerCurrentItem()));

	restoreHistory();
}

//virtual
WHistoryComboBox::~WHistoryComboBox()
{
	registerCurrentItem();
	saveHistory();
}

void WHistoryComboBox::restoreHistory()
{
	m_restoring = true;

	QSettings s;
	s.beginGroup(groupId());
	QStringList items = s.value(historyId()).toStringList();

	clearEditText();

	// DO NOT CLEAR! to preserve items that are inserted
	// programmatically before calling 'restoreHistory()'
	//
	// clear();
	foreach(const QString& item, items) {
		if (findText(item)<0) {
			addItem(item);
		}
	}
	QString lastItem = s.value(lastId(), QString("")).toString();
	if (!lastItem.isEmpty()) {
		setEditText(lastItem);
	}
	s.endGroup();

	m_restoring = false;
}

void WHistoryComboBox::saveHistory()
{
	QSettings s;
	s.beginGroup(groupId());
	QStringList items;
	for (int i = 0; i < count(); ++i) {
		items << itemText(i);
	}
	s.setValue(historyId(), items);
	s.setValue(lastId(), currentText());
	s.endGroup();
}

//virtual
void WHistoryComboBox::focusOutEvent(QFocusEvent* e)
{
	QComboBox::focusOutEvent(e);

	// registerCurrentItem();
}

//virtual
void WHistoryComboBox::keyPressEvent(QKeyEvent* e)
{
	QComboBox::keyPressEvent(e);

	if (e->key() == Qt::Key_Return) {
		registerCurrentItem();
		lineEdit()->selectAll();
	}
}

//virtual
bool WHistoryComboBox::eventFilter(QObject* obj, QEvent* e)
{
	if (obj==view()->viewport()) {
		if (e->type()==QEvent::MouseButtonPress) {
			QMouseEvent* me = (QMouseEvent*)e;
			if (me->button() == Qt::RightButton) {
				QModelIndex index = view()->indexAt(me->pos());
				if (index.isValid()) {
					removeItem(index.row());
					return true;
				}
			}
		}
	}
	return QComboBox::eventFilter(obj, e);
}

QString WHistoryComboBox::historyId() const
{
	return QLatin1String("/history");
}

QString WHistoryComboBox::lastId() const
{
	return QLatin1String("/last");
}

QString WHistoryComboBox::groupId() const
{
	return COMBO_HISTORY_GROUP + QLatin1String("/") + m_comboBoxName;
}

void WHistoryComboBox::registerCurrentItem()
{
	if (m_restoring) {
		return;
	}

	// add any newly edited item only if it is not in items
	QString c = currentText();
	if (c.size() > 0 && findText(c) < 0) {
		addItem(currentText());
	}
	model()->sort(modelColumn());
	saveHistory();
}
