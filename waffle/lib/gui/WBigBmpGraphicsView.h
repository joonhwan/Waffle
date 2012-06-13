#ifndef QGIGIMAGEGRAPHICSVIEW_H
#define QGIGIMAGEGRAPHICSVIEW_H

#include <QGraphicsView>

class QLabel;
class WBigBmpThreadedGraphicsScene;
class WDecoratedSimpleLabel;
class WTransparentProgressBar;

class WBigBmpGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	WBigBmpGraphicsView(QWidget *parent=0);
	~WBigBmpGraphicsView();

	void init(WBigBmpThreadedGraphicsScene* scene);
	void ZoomIn(void);
	void ZoomOut(void);
	void ResetZoom(void);
	void zoom(qreal factor, QPointF centerPoint);
	qreal zoomLevel(void) const;
protected slots:
	void updateProgress(int doneCount, int totalCount);
	void updatePixelInfo(int x, int y, QColor color);
protected:
	virtual void wheelEvent ( QWheelEvent * e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent * event);
	void scaleView(qreal scaleFactor);

	int m_panning;
	WDecoratedSimpleLabel* pixelInfoLabel;
	WTransparentProgressBar* renderProgress;
	WDecoratedSimpleLabel* renderStatus;
};

#endif // QGIGIMAGEGRAPHICSVIEW_H
