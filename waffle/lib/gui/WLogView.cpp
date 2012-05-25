#include "wlogview.h"
#include "common/logger/wlogevent.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QDebug>

WLogView::WLogView(QWidget* parent)
	: QPlainTextEdit(parent)
	, m_autoScroll(true)
{
	setMaximumBlockCount(100);
	setReadOnly(true);
	setWordWrapMode(QTextOption::NoWrap);

	QFont font("Bitstream Vera Sans Mono", 8);
	setFont(font);
}

//virtual
WLogView::~WLogView()
{
}

void WLogView::setMaxLines(int maxLines)
{
	QPlainTextEdit::setMaximumBlockCount(maxLines);
}

void WLogView::addLog(const WLogEvent& log)
{
	// set background color according to log level
	const char* bg = "#ffffff";
	switch (log.level) {
	case Wf::TraceLevel:
		bg = "#eeeeee";
		break;
	case Wf::DebugLevel:
		bg = "#dddddd";
		break;
	case Wf::WarnLevel:
		bg = "#CC0000";
		break;
	case Wf::ErrorLevel:
		bg = "#DD0000";
		break;
	case Wf::FatalLevel:
		bg = "#FF0000";
		break;
	}
	QColor bgColor(bg);

	QTextDocument* doc = document();
	QTextCursor cursor = textCursor();
	QTextCursor tmp(doc);
	tmp.beginEditBlock();
	tmp.movePosition(QTextCursor::End);

	QTextBlockFormat bf = cursor.blockFormat();
	bf.setBackground(QBrush(bg));
	cursor.setBlockFormat(bf);

	if (!doc->isEmpty())
		tmp.insertBlock(bf, cursor.charFormat());
	else
		tmp.setCharFormat(cursor.charFormat());

	// preserve the char format
	QTextCharFormat oldCharFormat = cursor.charFormat();

	tmp.insertText(log.logMessage());

	if (!cursor.hasSelection())
		cursor.setCharFormat(oldCharFormat);

	tmp.endEditBlock();

	if (m_autoScroll) {
		cursor.movePosition(QTextCursor::End);
		cursor.movePosition(QTextCursor::StartOfLine);
		setTextCursor(cursor);
	} else {
		// TODO CHECKME keep current line's visible position.?
		if (doc->blockCount() >= doc->maximumBlockCount()) {
			setTextCursor(cursor);
		}
	}
}

void WLogView::enableAutoScroll()
{
	qDebug() << "enable AutoScroll..";
	m_autoScroll = true;
}

void WLogView::disableAutoScroll()
{
	qDebug() << "disabled AutoScroll..";
	m_autoScroll = false;
}

//virtual
void WLogView::scrollContentsBy(int dx, int dy)
{
	QPlainTextEdit::scrollContentsBy(dx, dy);

	// if document's end is visible then "enable auto scroll"
	QScrollBar* vscroll = verticalScrollBar();
	int scv = vscroll->value();
	int lastPageStart = (vscroll->maximum() - vscroll->pageStep());
	// qDebug() << "scroll : " << scv << ", max: " << vscroll->maximum() << ", lastPageStart : " << lastPageStart;
	if (scv >= vscroll->maximum()) {
		enableAutoScroll();
	} else {
		disableAutoScroll();
	}
}
