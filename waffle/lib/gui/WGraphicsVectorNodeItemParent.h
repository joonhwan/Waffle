#pragma once

#include <QBrush>
#include <QGraphicsItem>
#include <QList>
#include <QPen>

class WGraphicsVectorNodeItem;
typedef QList<WGraphicsVectorNodeItem*> WGraphicsVectorNodeItemList;

class WGraphicsVectorNodeItemParentBase
{
public:
	WGraphicsVectorNodeItemParentBase();
	virtual ~WGraphicsVectorNodeItemParentBase();
	virtual void handleNodeDrag(WGraphicsVectorNodeItem* node,
								const QPointF& scenePos) = 0;
	virtual void applyNodeChange(WGraphicsVectorNodeItem* node,
								 QGraphicsItem::GraphicsItemChange change,
								 const QVariant& value) = 0;
	virtual QGraphicsItem* graphicsItem() = 0;
	virtual const QGraphicsItem* graphicsItem() const = 0;
	void enableEditing(bool enable = true);
	bool editing() const { return m_editing; }
	void setSelectedPen(const QPen& pen) { m_selectedPen = pen; }
	const QPen& selectedPen() const { return m_selectedPen; }
	void setSelectedBrush(const QBrush& pen) { m_selectedBrush = pen; }
	const QBrush& selectedBrush() const { return m_selectedBrush; }
	int selectedNodeCount() const;
	bool hasDraggingNode() const;
	void hideAllNodes() { setNodesVisibility(false); }
	void showAllNodes() { setNodesVisibility(true); }
	void deselectAllNodes() { setNodesSelection(false); }
	void selectAllNodes() { setNodesSelection(true); }
	void setNodesVisibility(bool visible=true);
	void setNodesSelection(bool selected=true);
	void addAllNodesToScene(QGraphicsScene* scene);
protected:
	bool m_editing;
	QPen m_selectedPen;
	QBrush m_selectedBrush;
	void* m_clientData;
	WGraphicsVectorNodeItemList m_nodes;
};

template<typename T>
class WGraphicsVectorNodeItemParent : public WGraphicsVectorNodeItemParentBase
									, public T
{
public:
	WGraphicsVectorNodeItemParent(QGraphicsItem* parent)
		: T(parent) { }
	virtual QGraphicsItem* graphicsItem() { return this; };
	virtual const QGraphicsItem* graphicsItem() const { return this; }
};
