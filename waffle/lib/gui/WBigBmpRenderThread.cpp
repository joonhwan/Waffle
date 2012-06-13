#include "WBigBmpRenderThread.h"
#include "WBigBmpImageFileLoader.h"
#include "WGraphicsCachedImageItem.h"

#include <QBuffer>
#include <QDebug>
#include <QQueue>
#include <QTextStream>
#include <limits.h>

namespace {

const int MAX_CACHE_COUNT = 50;

}

struct RenderQueueItem
{
	enum {
		CACHE_SCALED = 0,
		CACHE_RAW,
	};
	int code;
	int priority;
	WGraphicsCachedImageItem* graphicsItem;
};

class RenderQueue : protected QQueue<RenderQueueItem>
{
	typedef QQueue<RenderQueueItem> Container;
public:
	void push(const RenderQueueItem& newItem)
	{
		QMutexLocker locker(&m_queueLock);
		bool inserted = false;
		Container::iterator it=Container::begin();
		for(;it!=Container::end(); ++it) {
			const RenderQueueItem& item = *it;
			if(item.priority <= newItem.priority) {
				Container::insert(it, newItem);
				inserted = true;
				break;
			}
		}
		if(!inserted) {
			Container::push_back(newItem);
		}
		Q_ASSERT(Container::size());
		m_queueNotEmpty.wakeOne();
	}
	bool empty(void) const
	{
		QMutexLocker locker(&m_queueLock);
		return Container::empty();
	}
	bool tryPop(RenderQueueItem& item)
	{
		QMutexLocker locker(&m_queueLock);
		bool popped = false;
		if(!Container::empty()) {
			item = Container::dequeue();
			popped = true;
		}
		return popped;
	}
	bool waitAndPop(RenderQueueItem& item)
	{
		QMutexLocker locker(&m_queueLock);
		bool popped = false;
		while(Container::empty()) {
			m_queueNotEmpty.wait(&m_queueLock);
		}
		if(!Container::empty()) {
			item = Container::dequeue();
			popped = true;
		}
		return popped;
	}
	void clear(void)
	{
		QMutexLocker locker(&m_queueLock);
		Container::clear();
	}
private:
	mutable QMutex m_queueLock;
	QWaitCondition m_queueNotEmpty;
	bool closed;
};

WBigBmpRenderThread::WBigBmpRenderThread(QObject *parent)
	: QThread(parent)
	, m_image(0)
	, m_eventQueue(new RenderQueue)
	, m_cache()
{
}

WBigBmpRenderThread::~WBigBmpRenderThread()
{
}

void WBigBmpRenderThread::render(WGraphicsCachedImageItem* item, int priority)
{
	RenderQueueItem queueItem;
	queueItem.code = RenderQueueItem::CACHE_SCALED;
	queueItem.priority = priority;
	queueItem.graphicsItem = item;
	m_eventQueue->push(queueItem);
}

QImage WBigBmpRenderThread::cachedRawImageOf(WGraphicsCachedImageItem* item)
{
	QMutexLocker locker(&m_cacheLock);

	QImage image;
	CacheDataList::iterator it = m_cache.begin();
	for(; it!=m_cache.end(); ++it) {
		if((*it).item==item) {
			image = (*it).image;
			break;
		}
	}

	// qDebug() << "search cache out of " << m_cache.size() << " -- > null ? :" << image.isNull();
	return image;
}

void WBigBmpRenderThread::cacheRawImage(WGraphicsCachedImageItem* item)
{
	QRect r = item->region().toRect();
	QString log = QString(tr("caching image area[(x:%1,y:%2)-(w:%3,h:%4)]")).
				  arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
	emit renderStatusChanged(log);

	int recachePriority = 10000;
	RenderQueueItem queueItem;
	queueItem.code = RenderQueueItem::CACHE_RAW;
	queueItem.priority = recachePriority;
	queueItem.graphicsItem = item;
	m_eventQueue->push(queueItem);
}

bool WBigBmpRenderThread::startService(const QString& filePath)
{
	m_stopped = false;
	if(m_image) {
		delete m_image;
		m_image = 0;
	}
	m_image = new WBigBmpImageFileLoader;
	emit renderStatusChanged(tr("starting image loading..."));
	if(m_image->Open(filePath)) {
		reset();
		start();
	} else {
		emit renderStatusChanged(tr("unable to open image!"));
		delete m_image;
		m_image = 0;
	}
	return (m_image!=0);
}

void WBigBmpRenderThread::stopService(unsigned long waitTime)
{
	m_stopped = true;

	RenderQueueItem queueItem;
	queueItem.priority = INT_MAX;
	queueItem.graphicsItem = 0;
	m_eventQueue->push(queueItem);

	wait(waitTime);
	m_eventQueue->clear();
}

void WBigBmpRenderThread::reset(void)
{
	m_eventQueue->clear();
}

void WBigBmpRenderThread::resetItemCount(int count)
{
	emit renderProgress(0, count);
	m_itemCount = count;
	m_renderedItemCount = 0;
}

WBigBmpImageFileLoader* WBigBmpRenderThread::image(void)
{
	return m_image;
}

//virtual
void WBigBmpRenderThread::run(void)
{
	const int scaleDownRatio = 8; //256;

	forever {
		RenderQueueItem queueItem;
		if(m_eventQueue->waitAndPop(queueItem)) {
			WGraphicsCachedImageItem* item = queueItem.graphicsItem;
			switch(queueItem.code)
			{
			case RenderQueueItem::CACHE_SCALED:
				if(item && !item->thumbnailed()) {
					QSize scaleDownSize = item->boundingRect().size().toSize();
					scaleDownSize/=scaleDownRatio;
					QImage image = m_image->Image(item->region());
					// qDebug() << "region[" << item->region() << "] done!";
					emit renderedImage(item, image.scaled(scaleDownSize));
					emit renderProgress(++m_renderedItemCount, m_itemCount);
				}
				break;
			case RenderQueueItem::CACHE_RAW:
				if(item) {
					QSize scaleDownSize = item->boundingRect().size().toSize();
					scaleDownSize/=scaleDownRatio;
					QImage image = m_image->Image(item->region());
					// qDebug() << "region[" << item->region() << "] done!";
					addCache(item, image);
				}
				break;
			}
		}
		if(m_stopped) {
			break;
		}
	}
}

void WBigBmpRenderThread::addCache(WGraphicsCachedImageItem* item, QImage image)
{
	QMutexLocker locker(&m_cacheLock);

	if(m_cache.size() > MAX_CACHE_COUNT) {
		// qDebug() << "remove a cache data";
		m_cache.takeFirst();
	}
	CacheData data;
	data.item = item;
	data.image = image;
	m_cache.push_back(data);
	emit cachedImage(item);

	// qDebug() << "new cache is added. count : " << m_cache.size();
}

