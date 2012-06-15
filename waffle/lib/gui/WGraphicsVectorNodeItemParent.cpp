#include "WGraphicsVectorNodeItemParent.h"
#include "WGraphicsVectorNodeItem.h"
#include <QGraphicsScene>

namespace {

const int DefaultAlpha = 20;

}

WGraphicsVectorNodeItemParentBase::WGraphicsVectorNodeItemParentBase()
	: m_editing(false)
	, m_selectedPen(Qt::red)
	, m_selectedBrush(QColor(255,0,0, DefaultAlpha))
{
}

//virtual
WGraphicsVectorNodeItemParentBase::~WGraphicsVectorNodeItemParentBase()
{
	// NOTE: no need to delete m_nodes' items.
	//       they'll be delete'd in the QGraphicsItem's destructor.
}

void WGraphicsVectorNodeItemParentBase::enableEditing(bool enable)
{
	setNodesVisibility(enable);
	m_editing = enable;
}

int WGraphicsVectorNodeItemParentBase::selectedNodeCount() const
{
	int count = 0;
	foreach(WGraphicsVectorNodeItem* node, m_nodes) {
		if (node->isSelected()) {
			++count;
		}
	}
	return count;
}

bool WGraphicsVectorNodeItemParentBase::hasDraggingNode() const
{
	foreach(WGraphicsVectorNodeItem* node, m_nodes) {
		if (node->dragging()) {
			return true;
		}
	}
	return false;
}

void WGraphicsVectorNodeItemParentBase::setNodesVisibility(bool visible)
{
	foreach(WGraphicsVectorNodeItem* node, m_nodes) {
		node->setVisible(visible);
	}
}

void WGraphicsVectorNodeItemParentBase::setNodesSelection(bool selected)
{
	foreach(WGraphicsVectorNodeItem* node, m_nodes) {
		node->setSelected(selected);
	}
}

void WGraphicsVectorNodeItemParentBase::addAllNodesToScene(QGraphicsScene* scene)
{
	foreach(WGraphicsVectorNodeItem* node, m_nodes) {
		scene->addItem(node);
	}
}
