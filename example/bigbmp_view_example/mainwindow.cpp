#include "mainwindow.h"
#include "WBigBmpGraphicsView.h"
#include "gui/WBigBmpThreadedGraphicsScene.h"
#include "gui/WBigBmpImageFileLoader.h"
#include "gui/WGraphicsBigBmpImageItem.h"

#include <QAction>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_view(0)
	, m_scene(0)
{
	m_scene = new QGraphicsScene(this); // new WBigBmpThreadedGraphicsScene;
	m_view = new WBigBmpGraphicsView(this); // new WBigBmpGraphicsView;
	m_view->setScene(m_scene);

	m_imageItem = new WGraphicsBigBmpImageItem;
	m_scene->addItem(m_imageItem);

	connect(m_imageItem, SIGNAL(renderStatusChanged(const QString&)),
			m_view, SLOT(setText(const QString&)));
	connect(m_imageItem, SIGNAL(renderProgress(int,int)),
			m_view, SLOT(updateProgress(int,int)));
	connect(m_imageItem, SIGNAL(colorDetected(int,int,QColor)),
			m_view, SLOT(updatePixelInfo(int,int,QColor)));

	setCentralWidget(m_view);

	createActions();
	createMenus();

	// if(m_scene->loadImage("c:/temp/RECORD_110715_174326/left/GoldenImage.bmp")) {
	// 	m_view->resetTransform();
	// }
}

MainWindow::~MainWindow()
{
}

void MainWindow::open(void)
{
	QString filePath = QFileDialog::getOpenFileName(this,
													tr("Open a gray scale image"),
													"", // current dir
													tr("BMP File(*.bmp)"));
	if(!filePath.isEmpty()) {
		// if(m_scene->loadImage(filePath)) {
		// 	m_view->resetTransform();
		// }
		m_imageItem->loadImage(filePath);
	}
}

void MainWindow::exit(void)
{
}

void MainWindow::createActions()
{
	m_openAction = new QAction(tr("&Open"), this);
	m_openAction->setIcon(QIcon(":/QGigImageViewer/images/open.png"));
	m_openAction->setShortcut(tr("Ctrl+O"));
	m_openAction->setStatusTip(tr("Open a 8 bit grayscale image file in windows bmp format"));
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_exitAction = new QAction(tr("E&xit"), this);
	m_openAction->setIcon(QIcon(":/QGigImageViewer/images/exit.png"));
    m_exitAction->setShortcut(tr("Ctrl+Q"));
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus(void)
{
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);
}

