#pragma once

#include <QImage>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QScopedPointer>

class WBigBmpImageFileLoaderPrivate;

// gray scale image only
class WBigBmpImageFileLoader : public QObject
{
	Q_OBJECT
public:
	struct Data
	{
		QRectF sourceRegion;
		QImage image;
	};
	WBigBmpImageFileLoader(QObject *parent=0);
	~WBigBmpImageFileLoader();

	bool Open(const QString& imageFilePath);
	void Close(void);
	Data ImageData(const QRectF& _sourceRegion);
	QImage Image(const QRectF& _sourceRegion);
	bool Draw(QPainter* painter, const QRectF& _sourceRegion, const QRectF& targetRegion);
	QSize Size(void) const;
private:
	QScopedPointer<WBigBmpImageFileLoaderPrivate> m_impl;
};
