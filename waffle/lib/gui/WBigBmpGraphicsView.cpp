#include "WBigBmpGraphicsView.h"
#include "WBigBmpRenderThread.h"
#include "WBigBmpThreadedGraphicsScene.h"
#include "WDecoratedSimpleLabel.h"
#include "WTransparentProgressBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QVBoxLayout>

WBigBmpGraphicsView::WBigBmpGraphicsView(QWidget *parent)
	: WGraphicsView(parent)
{
	pixelInfoLabel = new WDecoratedSimpleLabel();
	QFont font = pixelInfoLabel->font();
	QString family = font.family();
	font.setPixelSize(20);
	font.setBold(true);
	pixelInfoLabel->setFont(font);
	pixelInfoLabel->showAlways(true);

	renderStatus = new WDecoratedSimpleLabel();
	renderStatus->setFont(font);
	renderStatus->setText("");
	renderStatus->setBorderPen(QPen(Qt::black, 0.5));
	renderStatus->setFillBrush(QBrush(Qt::red));

	renderProgress = new WTransparentProgressBar;
	renderProgress->setOpacity(0.0);
	renderProgress->setFormat(tr("Caching %p%"));

	QHBoxLayout* topLayout = new QHBoxLayout;
	topLayout->addWidget(pixelInfoLabel);
	topLayout->addStretch(1);

	QHBoxLayout* bottomLayout = new QHBoxLayout;
	bottomLayout->addWidget(renderProgress, 1);
	bottomLayout->addStretch(1);
	bottomLayout->addWidget(renderStatus);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(topLayout);
	layout->addStretch(1);
	layout->addLayout(bottomLayout);

	QMargins margins = layout->contentsMargins();
	int r = margins.right();
	int hw = horizontalScrollBar()->sizeHint().width() / 2;
	margins.setRight( r + hw );
	int b = margins.bottom();
	int vh = verticalScrollBar()->sizeHint().height() / 2;
	margins.setBottom( b + vh );
	layout->setContentsMargins(margins);
	setLayout(layout);
}

WBigBmpGraphicsView::~WBigBmpGraphicsView()
{

}

void WBigBmpGraphicsView::init(WBigBmpThreadedGraphicsScene* scene)
{
	this->setScene(scene);
	WBigBmpRenderThread* thread = scene->thread();

	connect(thread, SIGNAL(renderStatusChanged(const QString&)),
			renderStatus, SLOT(setText(const QString&)));
	connect(thread, SIGNAL(renderProgress(int,int)),
			SLOT(updateProgress(int,int)));
	connect(scene, SIGNAL(colorDetected(int,int,QColor)),
			SLOT(updatePixelInfo(int,int,QColor)));
}


void WBigBmpGraphicsView::updateProgress(int doneCount, int totalCount)
{
	if(doneCount==0) {
		renderProgress->setOpacity(1.);
	}

	renderProgress->setMaximum(totalCount);
	renderProgress->setValue(doneCount);
}

void WBigBmpGraphicsView::updatePixelInfo(int x, int y, QColor color)
{
	QString info = QString(tr("Pixel(%0,%1)-%2")).
				   arg(x).arg(y).arg(color.name());
	pixelInfoLabel->setText(info);
}
