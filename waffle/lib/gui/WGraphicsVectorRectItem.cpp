#include "WGraphicsVectorRectItem.h"
#include "WGraphicsVectorNodeItem.h"
#include "WGraphicsUtil.h"
#include <QPainter>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

namespace {

const qreal margin = 10;

enum {
	kTopLeft = 0,
	kTop,
	kTopRight,
	kRight,
	kBottomRight,
	kBottom,
	kBottomLeft,
	kLeft,

	kNodeCount
};

}


WGraphicsVectorRectItem::WGraphicsVectorRectItem(QGraphicsItem* parent)
	: WGraphicsVectorNodeItemParent<QGraphicsRectItem>(parent)
{
	QRectF initNullRect = QRectF(); // null rect
	m_nodes.clear();
	for (int i = 0; i < kNodeCount; ++i) {
		m_nodes << new WGraphicsVectorNodeItem(this);
	}
	m_nodes[kTopLeft]->setHint(kTopLeft);
	m_nodes[kTopRight]->setHint(kTopRight);
	m_nodes[kBottomRight]->setHint(kBottomRight);
	m_nodes[kBottomLeft]->setHint(kBottomLeft);
	m_nodes[kLeft]->setHint(kLeft);
	m_nodes[kRight]->setHint(kRight);
	m_nodes[kTop]->setHint(kTop);
	m_nodes[kBottom]->setHint(kBottom);

	m_nodes[kTopLeft]->setEditCursor(Qt::SizeFDiagCursor);
	m_nodes[kTopRight]->setEditCursor(Qt::SizeBDiagCursor);
	m_nodes[kBottomRight]->setEditCursor(Qt::SizeBDiagCursor);
	m_nodes[kBottomLeft]->setEditCursor(Qt::SizeFDiagCursor);
	m_nodes[kLeft]->setEditCursor(Qt::SizeHorCursor);
	m_nodes[kRight]->setEditCursor(Qt::SizeHorCursor);
	m_nodes[kTop]->setEditCursor(Qt::SizeVerCursor);
	m_nodes[kBottom]->setEditCursor(Qt::SizeVerCursor);

	setRect(initNullRect);
	setFlags(QGraphicsItem::ItemIsSelectable
			 | QGraphicsItem::ItemIsFocusable
			 | QGraphicsItem::ItemSendsGeometryChanges
			 | QGraphicsItem::ItemIsMovable
			 // | QGraphicsItem::ItemSendsGeometryChanges
		);
	setAcceptHoverEvents(true);
}

//virtual
WGraphicsVectorRectItem::~WGraphicsVectorRectItem()
{
}

//virtual
QRectF WGraphicsVectorRectItem::boundingRect() const
{
	QRectF br = QGraphicsRectItem::boundingRect();
	// qreal hns = m_nodes[kTopLeft]->nodeSize()/2.;
	// br.adjust(-hns,-hns,hns,hns);
	return br;
}

//virtual
QPainterPath WGraphicsVectorRectItem::shape() const
{
	qDebug() << "WGraphicsVectorRectItem::shape()";
	QPainterPath path = QGraphicsRectItem::shape();
	foreach(const WGraphicsVectorNodeItem* node, m_nodes) {
		path.addPath(mapFromItem(node, node->shape()));
	}
	return path;
}

//virtual
void WGraphicsVectorRectItem::paint(QPainter *painter,
									const QStyleOptionGraphicsItem *option,
									QWidget *widget)
{
	QRectF r = rect();
	if (option->state & QStyle::State_Selected) {
		painter->setPen(selectedPen());
		painter->setBrush(selectedBrush());
	} else {
		painter->setPen(pen());
		painter->setBrush(brush());
	}
	painter->drawRect(r);
}

//virtual
void WGraphicsVectorRectItem::handleNodeDrag(WGraphicsVectorNodeItem* node, const QPointF& scenePos)
{
	QRectF rect =  this->rect();
	QPointF nodeScenePos = node->mapFromScene(scenePos);

	// QRectF oldRect = rect;
	// QString cornerName;
	switch (node->hint())
	{
	case kTopLeft:
		// cornerName = "UL";
		if (nodeScenePos.x() < rect.right()
			&& nodeScenePos.y() < rect.bottom()) {
			rect.setTopLeft(nodeScenePos);
		}
		break;
	case kTopRight:
		// cornerName = "UR";
		if (nodeScenePos.x() > rect.left()
			&& nodeScenePos.y() < rect.bottom()) {
			rect.setTopRight(nodeScenePos);
		}
		break;
	case kBottomLeft:
		// cornerName = "LR";
		if (nodeScenePos.x() > rect.left()
			&& nodeScenePos.y() > rect.top()) {
			rect.setBottomRight(nodeScenePos);
		}
		break;
	case kBottomRight:
		// cornerName = "LL";
		if (nodeScenePos.x() < rect.right()
			&& nodeScenePos.y() > rect.top()) {
			rect.setBottomLeft(nodeScenePos);
		}
		break;

	case kLeft:
		if (nodeScenePos.x() < rect.right()) {
			rect.setLeft(nodeScenePos.x());
		}
		break;
	case kRight:
		if (nodeScenePos.x() > rect.left()) {
			rect.setRight(nodeScenePos.x());
		}
		break;
	case kTop:
		if (nodeScenePos.y() < rect.bottom()) {
			rect.setTop(nodeScenePos.y());
		}
		break;
	case kBottom:
		if (nodeScenePos.y() > rect.top()) {
			rect.setBottom(nodeScenePos.y());
		}
		break;
	}
	setRect(rect);

	adjustNodePosition();

	// qDebug() << "NodeDrag[" << cornerName << "]"
	// 		 << "(scene pos=" << scenePos << ", "
	// 		 << "item pos=" << itemPos << ") : " << oldRect
	// 		 << " -> " << rect << "(rect():" << this->rect() << ")";
}

//virtual
void WGraphicsVectorRectItem::applyNodeChange(WGraphicsVectorNodeItem* node,
											  GraphicsItemChange change,
											  const QVariant& value)
{
	switch (change)
	{
	case QGraphicsItem::ItemPositionChange:
		break;
	case QGraphicsItem::ItemSelectedChange:
		// {
		// 	bool selecting = value.toBool();
		// 	this->setSelected(selecting);
		// }
		break;
	}
}

QVariant WGraphicsVectorRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QVariant reaction = QAbstractGraphicsShapeItem::itemChange(change, value);

	WGraphicsVectorNodeItem* node = 0;
	switch (change)
	{
	case QGraphicsItem::ItemPositionHasChanged:
		adjustNodePosition();
		break;
	case QGraphicsItem::ItemSelectedHasChanged:
		{
			// show/hide all its nodes according to current state
			bool deselected = !value.toBool();
			if (deselected && !hasDraggingNode()) {
				enableEditing(false);
			}
		}
		break;
	case QGraphicsItem::ItemSceneChange:
		adjustNodePosition();
		// addAllNodesToScene(value.value<QGraphicsScene*>());
		break;
	}

	return reaction;
}

//virtual
void WGraphicsVectorRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	// qDebug() << "WGraphicsVectorRectItem::hoverEnterEvent";
	setCursor(Qt::OpenHandCursor);
	enableEditing(true);
}

//virtual
void WGraphicsVectorRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	// qDebug() << "WGraphicsVectorRectItem::hoverLeaveEvent";
	if (!isSelected() && !selectedNodeCount()) {
		enableEditing(false);
	}
}

//virtual
void WGraphicsVectorRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	setCursor(Qt::OpenHandCursor);
}

//virtual
void WGraphicsVectorRectItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	setCursor(Qt::ClosedHandCursor);
	QGraphicsRectItem::mousePressEvent(e);
}

//virtual
void WGraphicsVectorRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
	QGraphicsRectItem::mouseReleaseEvent(e);
	setCursor(Qt::OpenHandCursor);
}

//virtual
void WGraphicsVectorRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	QGraphicsRectItem::mouseMoveEvent(e);
}

void WGraphicsVectorRectItem::adjustNodePosition()
{
	// make nodes follow corner
	QRectF r = rect();
	m_nodes[kTopLeft]->setNodePos(r.topLeft());
	m_nodes[kTopRight]->setNodePos(r.topRight());
	m_nodes[kBottomLeft]->setNodePos(r.bottomRight());
	m_nodes[kBottomRight]->setNodePos(r.bottomLeft());

	QPointF c = r.center();
	m_nodes[kLeft]->setNodePos(QPointF(r.left(), c.y()));
	m_nodes[kRight]->setNodePos(QPointF(r.right(), c.y()));
	m_nodes[kTop]->setNodePos(QPointF(c.x(), r.top()));
	m_nodes[kBottom]->setNodePos(QPointF(c.x(), r.bottom()));
}
