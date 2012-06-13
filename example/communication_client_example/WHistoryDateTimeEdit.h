#pragma once

#include <QDateTimeEdit>

class WHistoryDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    WHistoryDateTimeEdit(const QString& name, QWidget* parent=0);
    virtual ~WHistoryDateTimeEdit();
signals:
public slots:
	void restoreHistory();
	void saveHistory();
protected:
	virtual void focusOutEvent(QFocusEvent* e);
	QString groupId();
private:
	QString m_name;
};
