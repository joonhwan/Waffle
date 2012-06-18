#pragma once

#include "gui/WGraphicsVectorNodeItemParent.h"
#include <QAbstractGraphicsShapeItem>
#include <QBrush>
#include <QPen>

// ���������� Scene������ Rect ������ ��������,
// WGraphicsVectorRectItem::rect() �� �ϸ� �ȵȴ�. �̰��� item��ǥ.
// mapToScene()�� ����� scene��ǥ��� ������ ��ȯ�޾ƾ� �Ѵ�.  ��
// �̰��� QPolygonF�̹Ƿ�, 4���� ������ ��� ���ؼ���
// QPolygonF::boundingRect()�Լ��� ȣ���ؾ� �Ѵ�.
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
