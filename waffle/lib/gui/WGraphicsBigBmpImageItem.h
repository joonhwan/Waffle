#pragma once

#include "gui/WGraphicsCachedImageProvider.h"
#include <QGraphicsObject>

class WBigBmpRenderThread;

class WGraphicsBigBmpImageItem : public QGraphicsObject
							   , public WGraphicsCachedImageProvider
{
    Q_OBJECT
public:
    WGraphicsBigBmpImageItem(QGraphicsItem* parent=0);
    virtual ~WGraphicsBigBmpImageItem();
	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	virtual bool eventFilter(QObject* watched, QEvent* e);

	// WGraphicsCachedImageProvider interface
	virtual void queueImageLoading(WGraphicsCachedImageItem* item, int priority);
	virtual void queueCacheImageJob(WGraphicsCachedImageItem* item);
	virtual QImage cachedImage(WGraphicsCachedImageItem* item);
signals:
	void renderStatusChanged(const QString& status);
	void renderProgress(int doneCount, int requestCount);
	void renderedImage(WGraphicsCachedImageItem* item, QImage scaleDown);
	void colorDetected(int x, int y, QColor color);
public slots:
	void updateCachedImage(WGraphicsCachedImageItem* item, QImage pixmap);
	void updateCachedImage(WGraphicsCachedImageItem* item);
	void finished(void);
	bool loadImage(const QString& filePath);
protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	WBigBmpRenderThread* m_thread;
	QSizeF m_imageSizePixel;
};
