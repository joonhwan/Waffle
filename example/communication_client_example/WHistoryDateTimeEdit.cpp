#include "WHistoryDateTimeEdit.h"
#include <QSettings>

namespace {

const char* VALUE_KEY = "date";

}

WHistoryDateTimeEdit::WHistoryDateTimeEdit(const QString& name, QWidget* parent)
	: QDateTimeEdit(parent)
	, m_name(name)
{
	restoreHistory();
}

//virtual
WHistoryDateTimeEdit::~WHistoryDateTimeEdit()
{
	saveHistory();
}

void WHistoryDateTimeEdit::restoreHistory()
{
	QSettings s;
	s.beginGroup(groupId());
	this->setDateTime(s.value(VALUE_KEY).toDateTime());
	s.endGroup();
}

void WHistoryDateTimeEdit::saveHistory()
{
	QSettings s;
	s.beginGroup(groupId());
	s.setValue(VALUE_KEY, dateTime());
	s.endGroup();
}

//virtual
void WHistoryDateTimeEdit::focusOutEvent(QFocusEvent* e)
{
	saveHistory();
	QDateTimeEdit::focusOutEvent(e);
}

QString WHistoryDateTimeEdit::groupId()
{
	return QLatin1String("WHistoryDateTimeEdit") + QLatin1String("/") + m_name;
}
