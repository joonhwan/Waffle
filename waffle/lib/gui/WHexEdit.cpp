#include "WHexEdit.h"

WHexEdit::WHexEdit(QWidget* parent)
	: QHexEdit(parent)
{
	QFont appFont = QApplication::font();
	QWidget* vp = widget();
	QFont widgetFont = vp->font();

	QString family = widgetFont.family();

	widgetFont.setPointSize(appFont.pointSize());
	vp->setFont(widgetFont);

	this->setViewportMargins(0,0,0,0);
}

// virtual
WHexEdit::~WHexEdit()
{
}

//virtual
QSize WHexEdit::minimumSizeHint() const
{
	QSize hint = QHexEdit::minimumSizeHint();
	return hint.expandedTo(QSize(viewport()->size().width(),
								 0));
}
