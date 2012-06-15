#pragma once

#include <QGraphicsView>

class WGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    WGraphicsView(QWidget* parent=0);
    virtual ~WGraphicsView();
	qreal zoomLevel(void) const;
signals:
public slots:
	void ZoomIn(void);
	void ZoomOut(void);
	void ResetZoom(void);
	void zoom(qreal factor, const QPointF& centerPoint);
protected:
	virtual void wheelEvent(QWheelEvent * e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent * event);
	void scaleView(qreal scaleFactor);

protected:
	int m_panning;
};
