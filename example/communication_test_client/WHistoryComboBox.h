#pragma once

#include <QComboBox>

class WHistoryComboBox : public QComboBox
{
    Q_OBJECT
public:
    WHistoryComboBox(const QString& comboBoxName, QWidget* parent=0);
    virtual ~WHistoryComboBox();
signals:
public slots:
	void restoreHistory();
	void saveHistory();
private slots:
	void registerCurrentItem();
protected:
	virtual void focusOutEvent(QFocusEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual bool eventFilter(QObject* obj, QEvent* e);
	QString historyId() const;
	QString lastId() const;
	QString groupId() const;

	bool m_restoring;
	QString m_comboBoxName;
};
