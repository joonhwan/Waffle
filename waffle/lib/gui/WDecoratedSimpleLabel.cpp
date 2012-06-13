#include "WDecoratedSimpleLabel.h"
#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>

WDecoratedSimpleLabel::WDecoratedSimpleLabel(QWidget *parent)
	: QFrame(parent)
	, m_text("")
	, m_showAlways(false)
{
	m_borderPen = QPen(palette().text().color());
	m_fillBrush = QBrush(palette().window().color());
	m_opacity = 1.0;
	m_opaqueAnimation = new QPropertyAnimation(this, "opacity");
	m_opaqueAnimation->setDuration(2000);
	// m_opaqueAnimation->setStartValue(1.0);
	// m_opaqueAnimation->setEndValue(0.0);
	m_opaqueAnimation->setKeyValueAt(0, 1.0);
	m_opaqueAnimation->setKeyValueAt(0.5, 0.8);
	m_opaqueAnimation->setKeyValueAt(1, 0.);

	setText(tr(""));
}

WDecoratedSimpleLabel::~WDecoratedSimpleLabel()
{
	delete m_opaqueAnimation;
}

//virtual
QSize WDecoratedSimpleLabel::sizeHint() const
{
	return m_textPath.boundingRect().size().toSize();
}

//virtual
QSize WDecoratedSimpleLabel::minimumSizeHint() const
{
	return QSize(0,0);
}

QString WDecoratedSimpleLabel::text(void) const
{
	return m_text;
}

QPen WDecoratedSimpleLabel::borderPen(void) const
{
	return m_borderPen;
}

QBrush WDecoratedSimpleLabel::fillBrush(void) const
{
	return m_fillBrush;
}

qreal WDecoratedSimpleLabel::opacity(void) const
{
	return m_opacity;
}

void WDecoratedSimpleLabel::showAlways(bool enabled)
{
	m_showAlways = enabled;
}

void WDecoratedSimpleLabel::setText(const QString& text)
{
	static int ll = 0;

	bool changed = (m_text!=text);
	if(changed) {
		qDebug() << "setText[" << ll++ << "] : text = " << text;
		m_text = text;
		updatePath();
	}
	if(m_showAlways || changed) {
		qDebug() << "setText[" << ll++ << "] : restart animation...";
		m_opaqueAnimation->stop();
		m_opaqueAnimation->start();
	}
}

void WDecoratedSimpleLabel::setBorderPen(const QPen& pen)
{
	m_borderPen = pen;
	update();
}

void WDecoratedSimpleLabel::setFillBrush(const QBrush& brush)
{
	m_fillBrush = brush;
	update();
}

void WDecoratedSimpleLabel::setOpacity(qreal opacity)
{
	m_opacity = opacity;
	update();
}

//virtual
void WDecoratedSimpleLabel::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.setOpacity(m_opacity);

	drawFrame(&painter);

	painter.setRenderHints(painter.renderHints() | QPainter::Antialiasing | QPainter::TextAntialiasing);
	painter.setPen(m_borderPen);
	painter.setBrush(m_fillBrush);
	painter.drawPath(m_textPath);
}

//virtual
void WDecoratedSimpleLabel::mousePressEvent(QMouseEvent* e)
{
	__super::mousePressEvent(e);
}

void WDecoratedSimpleLabel::updatePath(void)
{
	QPainterPath path;
	path.addText(0, 0, font(), m_text);

	QRectF box = path.boundingRect();
	path.translate(-box.topLeft());

	m_textPath = path;

	updateGeometry();
	update();
}


