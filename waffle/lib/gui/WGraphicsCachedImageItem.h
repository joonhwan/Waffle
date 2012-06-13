#pragma once

#include <QBrush>
#include <QGraphicsItem>
#include <QImage>

class WGraphicsCachedImageItem;

class WGraphicsCachedImageProvider
{
public:
	// �̹����� �� ó�� �е��� ��û.
	virtual void queueImageLoading(WGraphicsCachedImageItem* item, int priority) = 0;
	// Cached�� �̹����� �����ǵ��� ��û.
	virtual void queueCacheImageJob(WGraphicsCachedImageItem* item) = 0;
	// Ȥ�� �̹� �о�������� �ְ�, �װ� Cache�� ���µǸ�, Cached Image�� ��ȯ.
	// (cached�� �κ��� ���ٸ�, null image�� ��ȯ)
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
