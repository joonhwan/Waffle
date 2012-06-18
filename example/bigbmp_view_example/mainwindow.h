#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
// class WBmpGraphicsView;
// class WBigBmpThreadedGraphicsScene;
class WGraphicsView;
class QGraphicsScene;
class WGraphicsBigBmpImageItem;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent=0);
	~MainWindow();

protected slots:
	void open(void);
	void exit(void);

protected:
	void createActions();
    void createMenus();

	//WBigBmpGraphicsView* m_view;
	WGraphicsView* m_view;
	//WBigBmpThreadedGraphicsScene* m_scene;
	QGraphicsScene* m_scene;
	WGraphicsBigBmpImageItem* m_imageItem;

	QAction *m_openAction;
	QAction *m_exitAction;

	QMenu* m_fileMenu;
};

#endif // MAINWINDOW_H
