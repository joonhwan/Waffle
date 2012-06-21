#include "MainWindow.h"
#include "gui/WGraphicsView.h"
#include "gui/WGraphicsVectorRectItem.h"
#include "gui/WGraphicsVectorNodeItem.h"
#include "gui/UserInitArea.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	m_view = new WGraphicsView();
	m_scene = new QGraphicsScene(this);
	m_scene->setSceneRect(QRectF(0,0,1000,800));
	m_view->setScene(m_scene);

	QMenu* fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(tr("E&xit"), this, SLOT(close()));

	QMenu* editMenu = new QMenu(tr("&Edit"), this);
	editMenu->addAction(tr("Add &Rectangle"), this, SLOT(addRectangle()));

	QMenuBar* menuBar = this->menuBar();
	menuBar->addMenu(fileMenu);
	menuBar->addMenu(editMenu);

	setCentralWidget(m_view);
}

//virtual
MainWindow::~MainWindow()
{
}

void MainWindow::addRectangle()
{
	WGraphicsVectorRectItem* item = new WGraphicsVectorRectItem;
	item->setRect(QRectF(100,50,50,50));
	m_scene->addItem(item);
	QPointF pos = item->pos();

	WGraphicsVectorRectItem* item2 = new WGraphicsVectorRectItem;
	item2->setRect(QRectF(200,50,50,50));
	m_scene->addItem(item2);
	QPointF pos2 = item2->pos();

	QGraphicsRectItem* r = new QGraphicsRectItem(150,100,50,50);
	r->setBrush(Qt::red);
	m_scene->addItem(r);
	QPointF posR = r->pos();

	WGraphicsVectorNodeItem* node = new WGraphicsVectorNodeItem(0);
	node->setNodePos(QPointF(300,300));
	node->setVisible(true);
	m_scene->addItem(node);

	QRectF bb(250,100,50,100);
	QPolygonF poly(bb);
	UserInitArea* uia = new UserInitArea(0, poly);
	uia->setBrush(QColor(255,255,255,10));
	m_scene->addItem(uia);
}
