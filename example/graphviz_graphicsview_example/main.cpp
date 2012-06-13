#include "gui/graphviz/GVGraph.h"
#include <QDir>
#include <QtGui/QApplication>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	GVGraph graph("SIS");
	graph.addNodes(QStringList()
				   << "IPC1"
				   << "CAMERA1"
				   << "IPC2"
				   << "CAMERA2"
				   << "MPC"
				   << "VEC"
		);
	graph.addEdge("IPC1", "CAMERA1");
	graph.addEdge("IPC2", "CAMERA2");
	graph.addEdge("IPC1", "MPC");
	graph.addEdge("IPC2", "MPC");
	graph.addEdge("MPC", "VEC");
	graph.addEdge("VEC", "MPC");

	QGraphicsScene scene;
	QGraphicsView view;
	view.setScene(&scene);

	foreach(const GVNode& node, graph.nodes()) {
		QGraphicsEllipseItem* item = new QGraphicsEllipseItem(node.boundBox());
		scene.addItem(item);
	}
	foreach(const GVEdge& edge, graph.edges()) {
		QGraphicsPathItem* item = new QGraphicsPathItem;
		item->setPath(edge.path);
		scene.addItem(item);
	}

	view.show();

	return a.exec();
}
