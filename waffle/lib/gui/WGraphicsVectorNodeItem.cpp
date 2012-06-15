#include "WGraphicsVectorNodeItem.h"
#include <QBrush>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

WGraphicsVectorNodeItem::WGraphicsVectorNodeItem(WGraphicsVectorNodeItemParentBase* parent,
												 int hint)
	: QGraphicsRectItem(0)
	, m_nodeParent(parent)
	, m_size(10.0)
	, m_hint(hint)
	, m_dragging(false)
{
	if (parent) {
		// let this node be children of parent's graphics item.
		setParentItem(parent->graphicsItem());
	}
	setVisible(false); // by default
	setBrush(QBrush(Qt::black, Qt::SolidPattern));
	setPen(QPen()); // null pen
	setPos(QPointF()); // null position

	// do not enable itemIsMovable. we'll handle this ourselves
	setFlags(QGraphicsItem::ItemSendsGeometryChanges
			 | QGraphicsItem::ItemIsSelectable
			 // | QGraphicsItem::ItemSendsGeometryChanges
			 // | QGraphicsItem::ItemIsMovable
		);
	setAcceptHoverEvents(true);
}

//virtual
WGraphicsVectorNodeItem::~WGraphicsVectorNodeItem()
{
}

//virtual
void WGraphicsVectorNodeItem::paint(QPainter *painter,
									const QStyleOptionGraphicsItem *option,
									QWidget *widget)
{
	painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(rect());
	// do not selected state.
}

QPointF WGraphicsVectorNodeItem::pos() const
{
	return rect().center();
}

void WGraphicsVectorNodeItem::setNodePos(const QPointF& pos)
{
	this->setRect(QRectF(pos - QPointF(m_size/2.,m_size/2),
						 QSizeF(m_size,m_size)));
}

void WGraphicsVectorNodeItem::setX(qreal posX)
{
	QPointF pos = rect().center();
	pos.setX(posX);
	setPos(pos);
}

void WGraphicsVectorNodeItem::setY(qreal posY)
{
	QPointF pos = rect().center();
	pos.setY(posY);
	setPos(pos);
}

void WGraphicsVectorNodeItem::setNodeSize(qreal size)
{
	QPointF pos = rect().center();
	setNodePos(pos);
}

//static
QList<WGraphicsVectorNodeItem*>
WGraphicsVectorNodeItem::createNodesOn(const QPolygonF& poly,
									   WGraphicsVectorNodeItemParentBase* parent)
{
	QList<WGraphicsVectorNodeItem*> nodes;
	foreach(const QPointF& point, poly) {
		WGraphicsVectorNodeItem* node = new WGraphicsVectorNodeItem(parent);
		node->setPos(point);
		nodes << node;
	}
	return nodes;
}

//virtual
void WGraphicsVectorNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
	qDebug() << "Node Item: hover enter.";
	// setBrush(QBrush(Qt::red, Qt::SolidPattern));
	setCursor(m_cursor);
}

//virtual
void WGraphicsVectorNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
	qDebug() << "Node Item: hover leave.";
	// setBrush(QBrush(Qt::black, Qt::SolidPattern));
}

//virtual
void WGraphicsVectorNodeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
	qDebug() << "Node Item: hover move.";
}

//virtual
void WGraphicsVectorNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	// qDebug() << "Node Item: mouse pressed.";

	// before doing QGraphicsRectItem::mousePressEvent();
	m_dragging = true;
	m_dragBeginScenePos = e->scenePos();

	QGraphicsRectItem::mousePressEvent(e);

	if (m_nodeParent) {
		m_nodeParent->graphicsItem()->setSelected(true);
	}
}

//virtual
void WGraphicsVectorNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
	// qDebug() << "Node Item: mouse released.";
	QGraphicsRectItem::mouseReleaseEvent(e);
	m_dragging = false;

	// return selection to parent(to keep editing node...)
	if (m_nodeParent) {
		m_nodeParent->graphicsItem()->setSelected(true);
	}
}

//virtual
void WGraphicsVectorNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	// qDebug() << "Node Item: mouse moved :"
	// 		 << "pos:" << e->scenePos() << ", "
	// 		 << "(last:" << e->lastScenePos() << ")";
	QGraphicsRectItem::mouseMoveEvent(e);

	// let node's parent know dragging
	if (m_dragging && m_nodeParent) {
		// do not move node here. let parent shape handle it.
		m_nodeParent->handleNodeDrag(this,
									 e->scenePos());
	}
}

//virtual
QVariant WGraphicsVectorNodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QVariant v = QGraphicsRectItem::itemChange(change, value);

	// notify its change to node parent
	if (m_nodeParent) {
		m_nodeParent->applyNodeChange(this, change, value);
	}

	return v;
}
