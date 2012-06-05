#include "wlogview.h"
#include "common/logger/wlogevent.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QDebug>

namespace {
const int MAX_LINECOUNT = 50000;
}

WLogView::WLogView(QWidget* parent)
	: QPlainTextEdit(parent)
	, m_autoScroll(true)
	, m_enforceNewline(false)
{
	setMaximumBlockCount(100);
	setReadOnly(true);
	setWordWrapMode(QTextOption::NoWrap);
	setUndoRedoEnabled(false);
	setMaxLineCount(MAX_LINECOUNT);

	QFont font("Bitstream Vera Sans Mono", 8);
	setFont(font);
}

//virtual
WLogView::~WLogView()
{
}

void WLogView::addLog(const WLogEvent& log)
{
	// set background color according to log level
	const char* bg = "#ffffff";
	int fontWeight = QFont::Normal;
	switch (log.level) {
	case Wf::TraceLevel:
		bg = "#eeeeee";
		break;
	case Wf::DebugLevel:
		bg = "#dddddd";
		break;
	case Wf::WarnLevel:
		bg = "#CC0000";
		fontWeight = QFont::Bold;
		break;
	case Wf::ErrorLevel:
		bg = "#DD0000";
		fontWeight = QFont::Bold;
		break;
	case Wf::FatalLevel:
		bg = "#FF0000";
		fontWeight = QFont::Bold;
		break;
	}
	QColor bgColor(bg);

	QTextCharFormat textFormat;
	// textFormat.setForeground(bgColor);
	textFormat.setBackground(bgColor);
	textFormat.setFontWeight(fontWeight);

	QString logLine = log.logMessage() + QLatin1Char('\n');
	addText(logLine, textFormat);
}

void WLogView::addText(const QString& text, const QTextCharFormat& textFormat)
{
	QTextDocument* doc = document();
	if (m_maxLineCount > 0 && doc->blockCount() > m_maxLineCount) {
		return;
	}

	const bool atBottom = isScrollbarAtBottom();

	QTextCursor cursor = QTextCursor(doc);
	cursor.movePosition(QTextCursor::End);
	cursor.beginEditBlock();
	{
		// cursor.insertBlock();
		QString textToAdd = doNewlineEnfocement(text);
		if (textFormat.isValid()) {
			cursor.insertText(textToAdd, textFormat);
		} else {
			cursor.insertText(textToAdd);
		}
	}
	cursor.endEditBlock();

	if (atBottom) {
		scrollToBottom();
	}
}

QString WLogView::doNewlineEnfocement(const QString &out)
{
    // m_scrollToBottom = true;
    QString s = out;
    if (m_enforceNewline) {
        s.prepend(QLatin1Char('\n'));
        m_enforceNewline = false;
    }

    if (s.endsWith(QLatin1Char('\n'))) {
        m_enforceNewline = true; // make appendOutputInline put in a newline next time
        s.chop(1);
    }

    return s;
}

void WLogView::setMaxLineCount(int count)
{
    m_maxLineCount = count;
    setMaximumBlockCount(m_maxLineCount);
}

void WLogView::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

// void WLogView::enableAutoScroll()
// {
// 	qDebug() << "enable AutoScroll..";
// 	m_autoScroll = true;
// }

// void WLogView::disableAutoScroll()
// {
// 	qDebug() << "disabled AutoScroll..";
// 	m_autoScroll = false;
// }

//virtual
void WLogView::scrollContentsBy(int dx, int dy)
{
	QPlainTextEdit::scrollContentsBy(dx, dy);

	// // if document's end is visible then "enable auto scroll"
	// QScrollBar* vscroll = verticalScrollBar();
	// int scv = vscroll->value();
	// int lastPageStart = (vscroll->maximum() - vscroll->pageStep());
	// // qDebug() << "scroll : " << scv << ", max: " << vscroll->maximum() << ", lastPageStart : " << lastPageStart;
	// if (scv >= vscroll->maximum()) {
	// 	enableAutoScroll();
	// } else {
	// 	disableAutoScroll();
	// }
}

bool WLogView::isScrollbarAtBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}
