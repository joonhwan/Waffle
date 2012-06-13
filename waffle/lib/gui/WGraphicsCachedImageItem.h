#pragma once

#include <QBrush>
#include <QGraphicsItem>
#include <QImage>

class WGraphicsCachedImageItem;

class WGraphicsCachedImageProvider
{
public:
	// 이미지를 맨 처음 읽도록 요청.
	virtual void queueImageLoading(WGraphicsCachedImageItem* item, int priority) = 0;
	// Cached된 이미지가 생성되도록 요청.
	virtual void queueCacheImageJob(WGraphicsCachedImageItem* item) = 0;
	// 혹시 이미 읽어들인적이 있고, 그게 Cache된 상태되면, Cached Image을 반환.
	// (cached된 부분이 없다면, null image를 반환)
	virtual QImage cachedImage(WGraphicsCachedImageItem* item) = 0;
};

class WGraphicsCachedImageItem : public QGraphicsItem
{
public:
	enum { Type = UserType + 1 };
	WGraphicsCachedImageItem(const QRectF& region = QRectF());
	virtual ~WGraphicsCachedImageItem(void);
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
					   QWidget* widget);
    virtual int type() const
    {
        return Type;
    }
	void setIndex(int index)
	{
		m_index = index;
	}
	void setBrush(const QBrush& brush)
	{
		m_brush = brush;
	}
	const QRectF& region(void) const
	{
		return m_region;
	}
	void setThumbnail(QPixmap thumbnail);
	bool thumbnailed(void) const
	{
		return m_thumbnailed;
	}
	QColor colorAt(QPointF scenePos);
protected:
	bool m_thumbnailed;
	int m_index;
	QBrush m_brush;
	QRectF m_region;
	QPixmap m_scaledImage;
	QImage m_scaledImageI;
};
