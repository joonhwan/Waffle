#include "WGraphicsBigBmpImageItem.h"
#include "WGraphicsCachedImageItem.h"
#include "WBigBmpImageFileLoader.h"
#include "WBigBmpRenderThread.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

WGraphicsBigBmpImageItem::WGraphicsBigBmpImageItem(QGraphicsItem* parent)
	: QGraphicsObject(parent)
{
	m_thread = new WBigBmpRenderThread(this);
	qRegisterMetaType<QPixmap>("QPixmap");
	connect(m_thread, SIGNAL(renderedImage(WGraphicsCachedImageItem*,QImage)),
			SLOT(updateCachedImage(WGraphicsCachedImageItem*,QImage)));
	connect(m_thread, SIGNAL(cachedImage(WGraphicsCachedImageItem*)),
			SLOT(updateCachedImage(WGraphicsCachedImageItem*)));

	connect(m_thread, SIGNAL(renderStatusChanged(const QString&)),
			this, SIGNAL(renderStatusChanged(const QString&)));
	connect(m_thread, SIGNAL(renderProgress(int,int)),
			this, SIGNAL(renderProgress(int,int)));
	// connect(scene, SIGNAL(colorDetected(int,int,QColor)),
	// 		this, SLOT(updatePixelInfo(int,int,QColor)));
}

//virtual
WGraphicsBigBmpImageItem::~WGraphicsBigBmpImageItem()
{
}

//virtual
void WGraphicsBigBmpImageItem::queueImageLoading(WGraphicsCachedImageItem* item, int priority)
{
	m_thread->render(item, priority);
}

//virtual
void WGraphicsBigBmpImageItem::queueCacheImageJob(WGraphicsCachedImageItem* item)
{
	m_thread->cacheRawImage(item);
}

//virtual
QImage WGraphicsBigBmpImageItem::cachedImage(WGraphicsCachedImageItem* item)
{
	return m_thread->cachedRawImageOf(item);
}

//virtual
QRectF WGraphicsBigBmpImageItem::boundingRect() const
{
	return QRectF(QPointF(0,0), m_imageSizePixel);
}

//virtual
QPainterPath WGraphicsBigBmpImageItem::shape() const
{
	QPainterPath path;
	path.addRect(boundingRect());
	return path;
}

//virtual
void WGraphicsBigBmpImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// nothing to draw itself.
	// all child item(=WGraphicsCachedImageItem object) will do its drawing tasks.
}

//virtual
bool WGraphicsBigBmpImageItem::eventFilter(QObject* watched, QEvent* e)
{
	switch (e->type())
	{
	// case QEvent::GraphicsSceneHoverMove:
	// case QEvent::GraphicsSceneHoverEnter:
	// case QEvent::GraphicsSceneHoverLeave:
	case QEvent::GraphicsSceneMouseMove:
		{
			QGraphicsSceneMouseEvent* mouseEvent = (QGraphicsSceneMouseEvent*)e;
			QPointF cursorScenePos = mouseEvent->scenePos() + QPointF(-0.5, -0.5);
			QList<QGraphicsItem*> itemsOver = scene()->items(cursorScenePos);
			foreach(QGraphicsItem* item, itemsOver) {
				if(WGraphicsCachedImageItem* tile=qgraphicsitem_cast<WGraphicsCachedImageItem*>(item)) {
					QColor color = tile->colorAt(cursorScenePos);
					QPoint p = cursorScenePos.toPoint();
					emit colorDetected(p.x(), p.y(), color);
				}
			}
		}
		break;
	}
	return QGraphicsObject::eventFilter(watched, e);
}

void WGraphicsBigBmpImageItem::updateCachedImage(WGraphicsCachedImageItem* item,
												 QImage image)
{
	// do it in gui thread
	QPixmap pixmap;
	pixmap.convertFromImage(image);
	item->setThumbnail(pixmap);
}

void WGraphicsBigBmpImageItem::updateCachedImage(WGraphicsCachedImageItem* item)
{
	item->update();
}

void WGraphicsBigBmpImageItem::finished(void)
{
	// something to do?
}

bool WGraphicsBigBmpImageItem::loadImage(const QString& filePath)
{
    bool done = false;

	m_thread->stopService();

	QList<QGraphicsItem*> allItems = childItems(); // this->items();
	foreach(QGraphicsItem* item, allItems) {
		delete item; //removeItem(item);
	}
	// if(m_roi) {
	// 	removeItem(m_roi);
	// 	delete m_roi;
	// 	m_roi = 0;
	// }
	QBrush brush[2] = {
		QBrush(Qt::red),
		QBrush(Qt::blue)
	};
	if(m_thread->startService(filePath) &&
	   m_thread->image()) {
		int index = 0;
		QSize imageSize = m_thread->image()->Size();
		int itemWidth = imageSize.width();///16;
		int itemHeight = 1024;//128*4;
		QList<WGraphicsCachedImageItem*> itemsCreated;
		for(int x=0; x<imageSize.width(); x+=itemWidth) {
			for(int y=0; y<imageSize.height(); y+=itemHeight) {
				QRectF region(x,y,
							  qMin(itemWidth, imageSize.width()-x), qMin(itemHeight, imageSize.height()-y));
				WGraphicsCachedImageItem* item = new WGraphicsCachedImageItem(this, region);
				item->setIndex(index);
				item->setBrush(brush[index%2]); ++index;
				// addItem(item);
				item->setParentItem(this);

				itemsCreated << item;
			}
		}

		m_thread->resetItemCount(itemsCreated.size());
		foreach(WGraphicsCachedImageItem* item, itemsCreated) {
			m_thread->render(item);
		}

		QColor brushColor = Qt::yellow;
		brushColor.setAlpha(100);
		// m_roi = addRect(QRectF());
		// m_roi->setBrush(brushColor);
		// m_roi->setPen(QPen());

		done = true;
	}

	return done;
}

//virtual
QVariant WGraphicsBigBmpImageItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change)
	{
	case QGraphicsItem::ItemSceneChange:
		if (scene()) {
			scene()->removeEventFilter(this);
		}
		break;
	case QGraphicsItem::ItemSceneHasChanged:
		if (scene()) {
			scene()->installEventFilter(this);
		}
		break;
	}

	QVariant reaction = QGraphicsObject::itemChange(change, value);
	return reaction;
}

