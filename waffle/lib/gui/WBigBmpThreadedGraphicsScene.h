#ifndef QTHREADEDGIGIMAGEGRAPHICSSCENE_H
#define QTHREADEDGIGIMAGEGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QImage>
#include <QObject>

class WBigBmpRenderThread;
class WGraphicsCachedImageItem;

struct ThumbnailWorkResult
{
	ThumbnailWorkResult()
		: valid(false)
		, item(0)
	{}
	bool valid;
	QImage image;
	WGraphicsCachedImageItem* item;
};

class WBigBmpThreadedGraphicsScene : public QGraphicsScene
{
	Q_OBJECT

public:
	WBigBmpThreadedGraphicsScene(QObject *parent=0);
	virtual ~WBigBmpThreadedGraphicsScene();
	WBigBmpRenderThread* thread()
	{
		return m_thread;
	}
Q_SIGNALS:
	void colorDetected(int x, int y, QColor color);
public Q_SLOTS:
	void updateCachedImage(WGraphicsCachedImageItem* item, QImage pixmap);
	void updateCachedImage(WGraphicsCachedImageItem* item);
	void finished(void);
	bool loadImage(const QString& filePath);
protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

	WBigBmpRenderThread* m_thread;
	QGraphicsRectItem* m_rubberBand;
	QGraphicsRectItem* m_roi;
	QPointF m_rubberBandOrigin;
};

#endif // QTHREADEDGIGIMAGEGRAPHICSSCENE_H
