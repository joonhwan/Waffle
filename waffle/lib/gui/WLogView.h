#pragma once

#include <QPlainTextEdit>

class WLogEvent;

class WLogView : public QPlainTextEdit
{
    Q_OBJECT
public:
    WLogView(QWidget* parent=0);
    virtual ~WLogView();
	void setMaxLineCount(int count);
    int maxLineCount() const { return m_maxLineCount; }
signals:
public slots:
	void addLog(const WLogEvent& log);
	void addText(const QString& text, const QTextCharFormat& textFormat = QTextCharFormat());
	// void enableAutoScroll();
	// void disableAutoScroll();
protected:
	bool m_enforceNewline;
	bool m_autoScroll;
	int m_columnCount;
	int m_maxLineCount;

	virtual void scrollContentsBy(int dx, int dy);
	QString doNewlineEnfocement(const QString &out);
	bool isScrollbarAtBottom() const;
	void scrollToBottom();
};
