#pragma once

#include <QThread>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QList>
#include <QScopedPointer>
#include <QWaitCondition>

class WBigBmpImageFileLoader;
class WGraphicsCachedImageItem;
class RenderQueue;
typedef QScopedPointer<RenderQueue> RenderQueuePtr;

class WBigBmpRenderThread : public QThread
{
	Q_OBJECT
	struct CacheData
	{
		WGraphicsCachedImageItem* item;
		QImage image;
	};
	typedef QList<CacheData> CacheDataList;
public:
	WBigBmpRenderThread(QObject *parent);
	~WBigBmpRenderThread();

	void render(WGraphicsCachedImageItem* item, int priority=0);
	QImage cachedRawImageOf(WGraphicsCachedImageItem* item);
	void cacheRawImage(WGraphicsCachedImageItem* item);
	bool startService(const QString& filePath);
	void stopService(unsigned long waitTime=ULONG_MAX);
	void reset(void);
	void resetItemCount(int count);
	WBigBmpImageFileLoader* image(void);
signals:
	void renderStatusChanged(const QString& status);
	void renderProgress(int doneCount, int requestCount);
	void renderedImage(WGraphicsCachedImageItem* item, QImage scaleDown);
	void cachedImage(WGraphicsCachedImageItem* item);
protected:
	virtual void run(void);
	void addCache(WGraphicsCachedImageItem* item, QImage image);

	bool m_stopped;
	WBigBmpImageFileLoader* m_image;
	QWaitCondition m_queueNotEmpty;
	RenderQueuePtr m_eventQueue;
	mutable QMutex m_cacheLock;
	CacheDataList m_cache;
	int m_itemCount;
	int m_renderedItemCount;
};
