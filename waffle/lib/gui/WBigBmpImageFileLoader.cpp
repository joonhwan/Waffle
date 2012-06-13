#include "WBigBmpImageFileLoader.h"
#include "internal/bmpdata.h"
#include "common/global/WMemoryFileMapper.h"
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QIODevice>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QRgb>
#include <QVector>

class WBigBmpImageFileLoaderPrivate
{
public:
	struct CachedItem
	{
		QRect region;
		QByteArray buffer;
	};
	mutable QMutex m_mutex;
	BITMAPINFOHEADER m_fileBitmapInfo;
	QVector<QRgb> m_colorTable;
	WMemoryFileMapper::FileSizeType m_fileOffestToImageBuffer;
	WMemoryFileMapper m_fileMapper;
	QList<CachedItem> m_cachedData;
	CachedItem EnsureMap(const QRect& sourceRegion);
};

WBigBmpImageFileLoaderPrivate::CachedItem
WBigBmpImageFileLoaderPrivate::EnsureMap(const QRect& requestedRegion)
{
	bool done = false;
	CachedItem mapped;
	do
	{
		bool flip = m_fileBitmapInfo.biHeight > 0;

		// cache to accomdate request region
		qint32 width = m_fileBitmapInfo.biWidth;
		qint32 rightMostPos = width - 1;
		qint32 height = qAbs(m_fileBitmapInfo.biHeight);
		qint32 bottomMostPos = height - 1;

		// clipping TODO CHECKME
		mapped.region.setLeft(0);
		mapped.region.setTop( qMax(requestedRegion.top()/1024 - 1, 0) * 1024 );
		mapped.region.setBottom( qMin( (requestedRegion.bottom()/1024 + 1)*1024 - 1, bottomMostPos) );
		mapped.region.setRight(rightMostPos);

		// TODO
		//
		// update cache for 'mapped.region'.
		//
		// note : it might not be possible to create ALL cached data
		//        for it.  In this case, client should call
		//        'EnsureMap' again for the uncached region.
		//

		if(!m_cachedData.empty()) {
			const CachedItem& cached = m_cachedData.first();
			if(cached.region.contains(requestedRegion)) {
				// qDebug() << "cache hits!";
				mapped = cached;
				done = true;
				break;
			}
		}

		qint32 bpp = 8;// (qint32)m_fileBitmapInfo.biBitCount;
		WMemoryFileMapper::FileSizeType stride = (width * (bpp / 8) + (sizeof(quint32)-1)) & ~(sizeof(quint32)-1);
		WMemoryFileMapper::FileSizeType offset = m_fileOffestToImageBuffer;
		if(flip)
		{
			WMemoryFileMapper::FileSizeType endPosition = offset + height * stride;
			offset =  (endPosition - stride) - mapped.region.bottom() * stride;
		}
		else
		{
			offset = offset + stride  * (WMemoryFileMapper::FileSizeType)mapped.region.top();
		}

		int mappedImageHeight = mapped.region.height();
		int len = (int)(stride  * mappedImageHeight);
		if(mapped.buffer.size() < len) {
			mapped.buffer.resize(len);
		}
		Q_ASSERT(mapped.buffer.size() >= len);

		// regen bitmap
		if(!m_fileMapper.CopyContent(offset, len, mapped.buffer))
		{
			mapped.buffer.clear();
			break;
		}

		m_cachedData.clear();
		m_cachedData.push_back(mapped);

		// qDebug() << "newly cached!";

	} while(0);

	return mapped;
}

WBigBmpImageFileLoader::WBigBmpImageFileLoader(QObject *parent)
	: QObject(parent)
	, m_impl(new WBigBmpImageFileLoaderPrivate)
{
	// init gray scale color index table
	for(int i=0; i<=0xFF; ++i) {
		m_impl->m_colorTable << qRgb(i,i,i);
	}
}

WBigBmpImageFileLoader::~WBigBmpImageFileLoader()
{

}

bool WBigBmpImageFileLoader::Open(const QString& imageFilePath)
{
	Close();

	bool done = false;

	do
	{
		QMutexLocker lock(&m_impl->m_mutex);

		// check file
		QFileInfo fileInfo(imageFilePath);
		if(!fileInfo.exists()) {
			break;
		}

		// probing file format
		QFile file(imageFilePath);
		if(!file.open(QIODevice::ReadOnly)) {
			break;
		}
		quint64 readLength = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
		QByteArray tempBuffer(readLength, 0);
		QDataStream stream(&file);
		char* p = tempBuffer.data();
		if(readLength!=stream.readRawData(p, readLength)) {
			break;
		}
		BITMAPFILEHEADER* bfh = (BITMAPFILEHEADER*)p;
		BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)(p + sizeof(BITMAPFILEHEADER));
		const char* tag = (const char*)&bfh->bfType;
		if(tag[0] != 'B' ||
		   tag[1] != 'M')
		{
			// ooops not a bmp file format!
			break;
		}
		if(bih->biCompression!=BI_RGB)
		{
			// we cannot handle any other format than non-compressed rgb format
			break;
		}

		// keep file bitmap information
		::memcpy(&m_impl->m_fileBitmapInfo, bih, sizeof(m_impl->m_fileBitmapInfo));
		m_impl->m_fileOffestToImageBuffer = bfh->bfOffBits;

		if(!m_impl->m_fileMapper.Open(imageFilePath)) {
			break;
		}

		done = true;
	} while(0);

	if(!done) {
		Close();
	}

	return done;
}

void WBigBmpImageFileLoader::Close(void)
{
	QMutexLocker lock(&m_impl->m_mutex);
	m_impl->m_fileMapper.Close();
	qMemSet(&m_impl->m_fileBitmapInfo, 0, sizeof(m_impl->m_fileBitmapInfo));
}

WBigBmpImageFileLoader::Data WBigBmpImageFileLoader::ImageData(const QRectF& _sourceRegion)
{
	WBigBmpImageFileLoader::Data imageData;

	do
	{
		QMutexLocker lock(&m_impl->m_mutex);

		QRectF sourceRegion = _sourceRegion.normalized();

		// if not cached
		WBigBmpImageFileLoaderPrivate::CachedItem cached
			= m_impl->EnsureMap(sourceRegion.toRect());
		if(cached.region.isNull()) {
			break;
		}

		// adjust source rect in relative coordinate
		sourceRegion.translate(-1*cached.region.topLeft());

		bool flip = m_impl->m_fileBitmapInfo.biHeight > 0;

		// if(flip)
		// {
		// 	// 'top' is actually bottom position
		// 	int sourceRegionHeight = cached.region.height();
		// 	sourceRegion.setTop(cached.region.height() - sourceRegion.bottom());
		// 	sourceRegion.setHeight(sourceRegionHeight);
		// }

		qint32 width = m_impl->m_fileBitmapInfo.biWidth;
		qint32 height = m_impl->m_fileBitmapInfo.biHeight;
		m_impl->m_fileBitmapInfo.biBitCount = 8;
		qint32 bpp = m_impl->m_fileBitmapInfo.biBitCount;
		qint32 stride = (width * (bpp / 8) + (sizeof(quint32)-1)) & ~(sizeof(quint32)-1);
		QSize cachedSize = QSize(cached.region.width(),
								 cached.region.height());

		QImage cachedImage((const uchar*)cached.buffer.constData(),
						   cached.region.width(),
						   cached.region.height(),
						   stride,
						   QImage::Format_Indexed8);
		if(flip) {
			imageData.image = cachedImage.mirrored();
		} else {
			imageData.image = cachedImage;
		}

		imageData.image.setColorTable(m_impl->m_colorTable);
		imageData.sourceRegion = sourceRegion;

	} while(0);

	return imageData;
}

QImage WBigBmpImageFileLoader::Image(const QRectF& _sourceRegion)
{
	Data data = ImageData(_sourceRegion);
	Q_ASSERT(_sourceRegion.size()==data.sourceRegion.size());
	// int x = _sourceRegion.left() - data.sourceRegion.left();
	// int y = _sourceRegion.top() - data.sourceRegion.top();
	// QSizeF size = _sourceRegion.size().toSize();
	// Q_ASSERT(x >= 0 && y >= 0);

	// qDebug() << " source region: " << _sourceRegion << " -> " << data.sourceRegion;
	return data.image.copy(data.sourceRegion.toRect());
	// return data.image;
}

bool WBigBmpImageFileLoader::Draw(QPainter* painter, const QRectF& _sourceRegion, const QRectF& targetRegion)
{
	bool done = false;

	do
	{
		WBigBmpImageFileLoader::Data data = ImageData(_sourceRegion);
		if(data.image.isNull()) {
			break;
		}

		painter->drawImage(targetRegion,
						   data.image,
						   data.sourceRegion);
	} while(0);

	if(!done)
	{
		// draw a special image for letting user know there is no
		// image buffer available.
	}

	return done;
}

QSize WBigBmpImageFileLoader::Size(void) const
{
	QMutexLocker lock(&m_impl->m_mutex);

	QSize size(m_impl->m_fileBitmapInfo.biWidth,
			   qAbs(m_impl->m_fileBitmapInfo.biHeight));
	return size;
}
