#pragma once

#include <QWidget>

class WIntensityProfileWidget : public QWidget
{
    Q_OBJECT
	enum {
		R = 0,
		G,
		B
	};
public:
    WIntensityProfileWidget(Qt::Orientation=Qt::Horizontal, QWidget* parent=0);
    virtual ~WIntensityProfileWidget();
	virtual QSize sizeHint() const;
signals:
public slots:
	void setOrientation(Qt::Orientation orientation);
	void setProfile(const QList<int>& profile);
	void setProfile(const QList<int>& profileR,
					const QList<int>& profileG,
					const QList<int>& profileB);
	void setScale(qreal scale);
	void setOffset(qreal offset);
	// void showRange(qreal begin=-1, qreal end=-1);
protected:
	virtual void paintEvent(QPaintEvent* e);
private:
	Qt::Orientation m_orientation;
	// qreal m_visibleBeginIndex;
	// qreal m_visibleEndIndex;
	qreal m_offset;
	qreal m_scale;
	int m_maxIntensity;
	QList<int> m_profile[3]; // or mono profile.

	void clear();
	void setProfile(const QList<int>& profile, int index);
};
