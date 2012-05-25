#pragma once

#include <QPlainTextEdit>

class WLogEvent;

class WLogView : public QPlainTextEdit
{
    Q_OBJECT
public:
    WLogView(QWidget* parent=0);
    virtual ~WLogView();
	void setMaxLines(int maxLines);
signals:
public slots:
	void addLog(const WLogEvent& log);
	void enableAutoScroll();
	void disableAutoScroll();
protected:
	bool m_autoScroll;
	int m_columnCount;

	virtual void scrollContentsBy(int dx, int dy);
};
