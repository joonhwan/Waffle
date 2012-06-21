#pragma once

#include "common/global/WNamespace.h"
#include "common/global/WHsm.h"

class WBigBmpRendererEventId : public QObject
{
	Q_OBJECT
	Q_ENUMS(Value)
public:
	enum Value {
		ENTRY_STATE = Wf::Entry,
		INIT_STATE = Wf::Init,
		EXIT_STATE = Wf::Exit,
		TERM_STATEMACHINE = Wf::Term,

		IMAGE_THUMBNAILING, // queueImageLoading
		IMAGE_CACHING,
	};
};

class WBigBmpRenderer : public WHsm
{
    Q_OBJECT
public:
    WBigBmpRenderer(QObject* parent=0);
    virtual ~WBigBmpRenderer();

	void requestThumbnailing(WGraphicsCachedImageItem* item);
	void requestCache(WGraphicsCachedImageItem* item);
	QImage tryGetCachedImage(WGraphicsCachedImageItem* item);
protected:
	State myTop(const WEvent* e);
	State notReady(const WEvent* e);
	State ready(const WEvent* e);
	State   idle(const WEvent* e);
	State   busy(const WEvent* e);

	struct CacheData
	{
		WGraphicsCachedImageItem* item;
		QImage image;
	};
	typedef QList<CacheData> CacheDataList;

	bool m_thumbnailing;
	QList<WGraphicsCachedImageItem*> m_items;
	CacheDataList m_cacheList;
};
