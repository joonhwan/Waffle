#pragma once

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
