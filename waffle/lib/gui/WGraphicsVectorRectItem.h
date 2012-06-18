#pragma once

#include "gui/WGraphicsVectorNodeItemParent.h"
#include <QAbstractGraphicsShapeItem>
#include <QBrush>
#include <QPen>

// 최종적으로 Scene에서의 Rect 영역을 얻으려면,
// WGraphicsVectorRectItem::rect() 로 하면 안된다. 이것은 item좌표.
// mapToScene()을 사용해 scene좌표계로 영역을 반환받아야 한다.  단
// 이것이 QPolygonF이므로, 4각형 영역을 얻기 위해서는
// QPolygonF::boundingRect()함수를 호출해야 한다.
class WGraphicsVectorRectItem : public WGraphicsVectorNodeItemParent<QGraphicsRectItem>
{
public:
	enum { Type = UserType + 1001};
    int type() const { return Type; }
    WGraphicsVectorRectItem(QGraphicsItem* parent=0);
    virtual ~WGraphicsVectorRectItem();

	// WGraphicsVectorNodeItemParent interface
	virtual void handleNodeDrag(WGraphicsVectorNodeItem* node, const QPointF& scenePos);
	virtual void applyNodeChange(WGraphicsVectorNodeItem* node, GraphicsItemChange change,const QVariant& value);
	// QGraphicsRectItem interface
	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* e);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
	void adjustNodePosition();
};
