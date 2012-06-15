#pragma once

#include "gui/WGraphicsVectorNodeItemParent.h"
#include <QCursor>
#include <QGraphicsRectItem>

class WGraphicsVectorAbstractItem;

class WGraphicsVectorNodeItem : public QGraphicsRectItem
{
public:
	enum { Type = UserType + 1000 };
	int type() const { return Type; }
    WGraphicsVectorNodeItem(WGraphicsVectorNodeItemParentBase* parent, int hint=0);
    virtual ~WGraphicsVectorNodeItem();
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	int hint() const { return m_hint; }
	QPointF pos() const;
	void setHint(int hint) { m_hint = hint; }
	void setNodePos(const QPointF& pos);
	void setX(qreal posX);
	void setY(qreal posY);
	qreal nodeSize() const { return m_size; }
	void setNodeSize(qreal size);
	void setEditCursor(QCursor cursor) { m_cursor = cursor; }
	bool dragging() const { return m_dragging; }
	static WGraphicsVectorNodeItemList createNodesOn(const QPolygonF& poly, WGraphicsVectorNodeItemParentBase* parent);
protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* e);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	WGraphicsVectorNodeItemParentBase* m_nodeParent;
	bool m_dragging;
	QPointF m_dragBeginScenePos;
	QCursor m_cursor;
	int m_hint;
	qreal m_size; // = width = height
};
