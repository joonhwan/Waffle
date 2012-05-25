#include "WIntensityProfileWidget.h"
#include <QPainter>
#include <QDebug>
#include <QtAlgorithms>

WIntensityProfileWidget::WIntensityProfileWidget(Qt::Orientation orientation, QWidget* parent)
	: m_orientation(orientation)
	, m_offset(0.)
	, m_scale(1.)
{
	clear();
}

//virtual
WIntensityProfileWidget::~WIntensityProfileWidget()
{
}

//virtual
QSize WIntensityProfileWidget::sizeHint() const
{
	if (m_orientation == Qt::Horizontal) {
		return QSize(10,50);
	} else {
		return QSize(50,10);
	}
}

void WIntensityProfileWidget::setOrientation(Qt::Orientation orientation)
{
	if (m_orientation != orientation) {
		m_orientation = orientation;
		updateGeometry();
	}
}

void WIntensityProfileWidget::setProfile(const QList<int>& profile)
{
	clear();
	setProfile(profile, R);
	update();
}

void WIntensityProfileWidget::clear()
{
	for (int c = 0; c < 3; ++c) {
		m_profile[c].clear();
	}
	m_maxIntensity = 0;
}

void WIntensityProfileWidget::setProfile(const QList<int>& profile, int index)
{
	m_profile[index] = profile;

	int max = m_maxIntensity;
	foreach (int v, m_profile[index]) {
		if ( v > max ) {
			max = v;
		}
	}
	m_maxIntensity = max;
}

void WIntensityProfileWidget::setProfile(const QList<int>& profileR,
										 const QList<int>& profileG,
										 const QList<int>& profileB)
{
	clear();
	setProfile(profileR, R);
	setProfile(profileG, G);
	setProfile(profileB, B);
	update();
}

void WIntensityProfileWidget::setScale(qreal scale)
{
	m_scale = scale;
	update();
}

void WIntensityProfileWidget::setOffset(qreal offset)
{
	m_offset = offset;
	update();
}

// void WIntensityProfileWidget::showRange(qreal begin, qreal end)
// {
// 	if (begin > end) {
// 		qSwap(begin, end);
// 	}
// 	m_visibleBeginIndex = begin;
// 	m_visibleEndIndex = end;
// 	update();
// }

//virtual
void WIntensityProfileWidget::paintEvent(QPaintEvent* e)
{
	QColor color[3] = {
		QColor(255,0,0),
		QColor(0,255,0),
		QColor(0,0,255)
	};
	QPainter painter(this);
	painter.setBrush(Qt::darkGray);
	painter.drawRect(QRect(QPoint(0,0),
						   size()));

	bool monoProfile = m_profile[G].empty() ||
					   m_profile[B].empty();
	int profileCount = monoProfile ? 1 : 3;

	// setup coord system
	QSizeF s = size();
	if (m_orientation == Qt::Horizontal) {
		painter.scale(m_scale,
					  s.height()/double (m_maxIntensity));
		painter.translate(QPointF(-m_offset,0));
		painter.fillRect(0,0,m_profile[0].size(),m_maxIntensity, Qt::black);
	} else {
		painter.scale(s.width()/double(m_maxIntensity),
					  m_scale);
		painter.translate(QPointF(0, -m_offset));
		painter.fillRect(0,0,m_maxIntensity, m_profile[0].size(), Qt::black);
	}

	// drawing profile
	if (monoProfile) {
		QList<int>& profile = m_profile[0];
		QColor fillColor = Qt::lightGray;
		for (int i = 0; i < profile.size(); ++i) {
			int value = profile[i];
			if (m_orientation == Qt::Horizontal) {
				painter.fillRect(i, m_maxIntensity-value, 1, value, fillColor);
			} else {
				painter.fillRect(0, i, value, 1,  fillColor);
			}
		}
	} else {
		for (int c = 0; c < 3; ++c) {
			QList<int>& profile = m_profile[c];
			QColor drawColor = color[c];
			painter.setPen(drawColor);
			for (int i = 1; i < profile.size(); ++i) {
				int preValue = profile[i-1];
				int value = profile[i];
				if (m_orientation == Qt::Horizontal) {
					// painter.fillRect(i, m_maxIntensity-value, 1, value, fillColor);
					painter.drawLine(i-1, m_maxIntensity-preValue, i, m_maxIntensity-value);
				} else {
					// painter.fillRect(0, i, value, 1,  fillColor);
					painter.drawLine(preValue, i-1, value, i);
				}
			}
		}
	}
}
