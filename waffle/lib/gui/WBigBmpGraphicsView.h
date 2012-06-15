#pragma once

#include "gui/WGraphicsView.h"

class QLabel;
class WBigBmpThreadedGraphicsScene;
class WDecoratedSimpleLabel;
class WTransparentProgressBar;

class WBigBmpGraphicsView : public WGraphicsView
{
	Q_OBJECT

public:
	WBigBmpGraphicsView(QWidget *parent=0);
	~WBigBmpGraphicsView();

	void init(WBigBmpThreadedGraphicsScene* scene);
protected slots:
	void updateProgress(int doneCount, int totalCount);
	void updatePixelInfo(int x, int y, QColor color);
protected:
	WDecoratedSimpleLabel* pixelInfoLabel;
	WTransparentProgressBar* renderProgress;
	WDecoratedSimpleLabel* renderStatus;
};
