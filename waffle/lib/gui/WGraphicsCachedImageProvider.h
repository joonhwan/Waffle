#pragma once

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
