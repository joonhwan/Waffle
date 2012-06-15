#pragma once

#include <QMainWindow>

class QGraphicsScene;
class WGraphicsView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    virtual ~MainWindow();
signals:
public slots:
	void addRectangle();
protected:
	WGraphicsView* m_view;
	QGraphicsScene* m_scene;
};
