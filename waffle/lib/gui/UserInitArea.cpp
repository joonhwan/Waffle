/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <iostream>


#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtCore/QRect>


#include "UserInitArea.h"
// #include "UserInitAreaDialog.h"

namespace UIA {

const int Alpha = 100;

}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::UserInitArea(int userIndex, const QPolygonF &polygon,  QGraphicsItem *parent) :
	QGraphicsPolygonItem(polygon, parent)
{
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	// setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setAcceptHoverEvents(true);
	m_isResizing = false;
	m_CurrentResizeHandle = UserInitArea::NO_CTRL_POINT;
	ctrlPointSize = 7.0f;
	m_GrayLevel = 0 + (255/16 * userIndex);

	m_Class = userIndex;
	m_Mu = 1.0;
	m_Sigma = 0.1;
	m_Gamma = 0.0;
	m_MinVariance = 20.0;
	m_LineWidth = 1.0;
	m_Visible = true;

	QPoint p = pos().toPoint();
	QRect b = boundingRect().toAlignedRect();
	m_UpperLeft[0] = b.x() + p.x();
	m_UpperLeft[1] = b.y() + p.y();
	m_LowerRight[0] = b.x() + p.x() + b.width();
	m_LowerRight[1] = b.y() + p.y() + b.height();

	QStringList colorNames = QColor::colorNames();
	setBrush(QBrush(QColor(colorNames[21])));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::~UserInitArea()
{
	//std::cout << "~UserInitArea" << std::endl;
}

#define READ_VALUE(prefs, var, ok, temp, default, type)	\
	ok = false;											\
	temp = prefs.value(#var).to##type(&ok);				\
	if (false == ok) {temp = default;}					\
	var = temp;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea* UserInitArea::NewUserInitArea(QSettings &prefs, int index, QGraphicsItem *parent)
{
	bool ok = false;
	int m_Class = index;
	int m_GrayLevel = 0;
	double m_Gamma = 0.0;
	double m_MinVariance = 20.0;
	double m_LineWidth = 1.0;
	QColor color;
	int r, g, b;
	unsigned int x, y;
	unsigned int width, height;
	QString group("UserInitArea-");
	group.append(QString::number(index));
	prefs.beginGroup(group);

	m_Class = prefs.value("Class").toInt(&ok);
	m_GrayLevel = prefs.value("GrayLevel").toInt(&ok);
	m_Gamma = prefs.value("Gamma").toDouble(&ok);
	m_MinVariance = prefs.value("MinVariance").toDouble(&ok);

	x = prefs.value("X").toInt(&ok);
	y = prefs.value("Y").toInt(&ok);
	width = prefs.value("Width").toInt(&ok);
	height = prefs.value("Height").toInt(&ok);

	QRectF rect(x, y, width, height);

	m_LineWidth = prefs.value("LineWidth").toInt(&ok);
	r = prefs.value("Red").toInt(&ok);
	g = prefs.value("Gree").toInt(&ok);
	b = prefs.value("Blue").toInt(&ok);

	QColor c = QColor(r, g, b, UIA::Alpha);
	prefs.endGroup();

	UserInitArea* uia = new UserInitArea(m_Class, rect, parent);
	uia->setEmMpmGrayLevel(m_GrayLevel);
	uia->setGamma(m_Gamma);
	uia->setMinVariance(m_MinVariance);
	uia->setLineWidth(m_LineWidth);
	uia->setColor(c);

	// GraphicsView related stuff
	// Line Color
	uia->setPen(QPen(QColor(225, 225, 225, UIA::Alpha)));
	// Fill Color
	uia->setBrush(QBrush(c));
	uia->setZValue(1);
	uia->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	return uia;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::writeSettings(QSettings &prefs)
{
	QString group("UserInitArea-");
	group.append(QString::number(m_Class));

	prefs.beginGroup(group);
	prefs.setValue("Class", m_Class);
	prefs.setValue("GrayLevel", m_GrayLevel);
	prefs.setValue("Gamma", m_Gamma);
	prefs.setValue("MinVariance", m_MinVariance);

	unsigned int x, y;
	unsigned int width, height;
	getUpperLeft(x, y);
	getLowerRight(width, height);
	width = width - x;
	height = height - y;
	prefs.setValue("X", x);
	prefs.setValue("Y", y);
	prefs.setValue("Width", width);
	prefs.setValue("Height", height);

	prefs.setValue("LineWidth", m_LineWidth);
	QRgb rgb = getColor().rgb();
	prefs.setValue("Red", qRed(rgb));
	prefs.setValue("Green", qGreen(rgb));
	prefs.setValue("Blue", qBlue(rgb));

	prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::getUpperLeft(unsigned int &x, unsigned int &y)
{
	QPoint p = pos().toPoint();
	QRect b = boundingRect().toAlignedRect();
	m_UpperLeft[0] = b.x() + p.x();
	m_UpperLeft[1] = b.y() + p.y();

	x = m_UpperLeft[0];
	y = m_UpperLeft[1];
}

//void UserInitArea::setLowerRight(unsigned int x, unsigned int y)
//{
//  m_LowerRight[0] = x;
//  m_LowerRight[1] = y;
//}


void UserInitArea::getLowerRight(unsigned int &x, unsigned int &y)
{
	QPoint p = pos().toPoint();
	QRect b = boundingRect().toAlignedRect();
	m_LowerRight[0] = b.x() + p.x() + b.width();
	m_LowerRight[1] = b.y() + p.y() + b.height();

	x = m_LowerRight[0];
	y = m_LowerRight[1];
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setMu(double mu)
{
	m_Mu = mu;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double UserInitArea::getMu()
{
	return m_Mu;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setSigma(double sigma)
{
	m_Sigma = sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double UserInitArea::getSigma()
{
	return m_Sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setGamma(double g)
{
	m_Gamma = g;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double UserInitArea::getGamma()
{
	return m_Gamma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setMinVariance(double g)
{
	m_MinVariance = g;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double UserInitArea::getMinVariance()
{
	return m_MinVariance;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setLineWidth(qreal w)
{
	m_LineWidth = w;
	emit fireUserInitAreaUpdated(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
qreal UserInitArea::getLineWidth()
{
	return m_LineWidth;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setVisible(bool visible)
{
	m_Visible = visible;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (m_Visible == false)
	{
		std::cout << "UserInitArea::paint Visible=false" << std::endl;
		return;
	}

	painter->setRenderHint(QPainter::Antialiasing, true);

	if (option->state & QStyle::State_HasFocus)
	{
		painter->setPen(QPen(QColor(255, 25, 25, UIA::Alpha), 3.0));
		painter->setBrush(brush());
	}
	else
	{
		painter->setPen(pen());
		painter->setBrush(brush());
	}

	painter->drawRect(boundingRect());
	if (option->state & QStyle::State_Selected)
	{
		float x = boundingRect().x();
		float y = boundingRect().y();
		float w = boundingRect().width();
		float h = boundingRect().height();

		painter->setPen(QPen(QColor(255, 25, 25, UIA::Alpha)));
		painter->setBrush( QBrush(QColor(255, 25, 25, UIA::Alpha)));
		//Upper Left
		painter->drawRect((int)x, (int)y, (int)ctrlPointSize, (int)ctrlPointSize);
		//Upper Right
		painter->drawRect((int)x + (int)w - (int)ctrlPointSize, (int)y, (int)ctrlPointSize, (int)ctrlPointSize);
		// Lower Right
		painter->drawRect((int)x + (int)w - (int)ctrlPointSize, (int)y + (int)h - (int)ctrlPointSize, (int)ctrlPointSize, (int)ctrlPointSize);
		// Lower Left
		painter->drawRect((int)x, (int)y + (int)h - (int)ctrlPointSize, (int)ctrlPointSize, (int)ctrlPointSize);
	}

	painter->setRenderHint(QPainter::Antialiasing, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event)
{
	if (!isSelected() && scene()) {
		scene()->clearSelection();
		setSelected(true);
	}

	propertiesSelectedItems(scene());
	//    else if (selectedAction == growAction)
	//        growSelectedItems(scene());
	//    else if (selectedAction == shrinkAction)
	//        shrinkSelectedItems(scene());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!isSelected() && scene()) {
        scene()->clearSelection();
        setSelected(true);
    }

    QMenu menu;

    QAction *propertiesAction = menu.addAction("Properties");
    menu.addSeparator();
    QAction *delAction = menu.addAction("Delete");

	//    QAction *growAction = menu.addAction("Grow");
	//    QAction *shrinkAction = menu.addAction("Shrink");

    QAction *selectedAction = menu.exec(event->screenPos());


    if (selectedAction == delAction)
        deleteSelectedItems(scene());
    else if (selectedAction == propertiesAction)
        propertiesSelectedItems(scene());
	//    else if (selectedAction == growAction)
	//        growSelectedItems(scene());
	//    else if (selectedAction == shrinkAction)
	//        shrinkSelectedItems(scene());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::propertiesSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase) {
			// UserInitAreaDialog about(NULL);
			// about.getUserInitAreaWidget()->setUserInitArea(itemBase);
			// about.exec();
			// emit itemBase->fireUserInitAreaUpdated(itemBase);
		}
    }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::duplicateSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
	//TODO This needs to be fully implemeted
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase)
			scene->addItem(itemBase->createNew(itemBase->m_size, itemBase->pos().x() + itemBase->m_size, itemBase->pos().y()));
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::deleteSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase)
		{
			emit itemBase->fireUserInitAreaAboutToDelete(itemBase);
			emit itemBase->fireUserInitAreaDeleted(itemBase);
			delete itemBase;
		}
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::deleteAllUserInitAreas(QGraphicsScene* scene)
{
	if (!scene)
		return;
	QList<QGraphicsItem *> selected;
	selected = scene->items();

	foreach (QGraphicsItem *item, selected)
	{
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase)
		{
			emit itemBase->fireUserInitAreaAboutToDelete(itemBase);
			emit itemBase->fireUserInitAreaDeleted(itemBase);
			delete itemBase;
		}
	}

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::growSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;
	//TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase)
		{
			itemBase->prepareGeometryChange();
			itemBase->m_size *= 2;
			if (itemBase->m_size > MAX_ITEM_SIZE)
				itemBase->m_size = MAX_ITEM_SIZE;
		}
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::shrinkSelectedItems(QGraphicsScene *scene)
{
    if (!scene)
        return;

    //TODO: This needs to be fully implemented
#if 0
    QList<QGraphicsItem *> selected;
    selected = scene->selectedItems();

    foreach (QGraphicsItem *item, selected)
    {
		UserInitArea *itemBase = qgraphicsitem_cast<UserInitArea *>(item);
		if (itemBase)
		{
			itemBase->prepareGeometryChange();
			itemBase->m_size /= 2;
			if (itemBase->m_size < MIN_ITEM_SIZE)
				itemBase->m_size = MIN_ITEM_SIZE;
		}
    }
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	static qreal z = 0.0;
	setZValue(z += 1.0);
	m_CurrentResizeHandle = isInResizeArea(event->pos());
	if (event->button() == Qt::LeftButton && m_CurrentResizeHandle != UserInitArea::NO_CTRL_POINT)
	{
		//  std::cout << "mousePressEvent m_isResizing = true" << std::endl;
		m_isResizing = true;
	}
	else
	{
		QGraphicsItem::mousePressEvent(event);
		emit fireUserInitAreaSelected(this);
	}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isResizing) {
        m_isResizing = false;
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
    emit fireUserInitAreaUpdated(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_isResizing)
	{
		//      std::cout << "mouseMoveEvent m_isResizing = true" << std::endl;
		QPointF lastPos = event->lastScenePos();
		QPointF pos = event->scenePos();
		float deltaX = pos.x() - lastPos.x();
		float deltaY = pos.y() - lastPos.y();
		float x = boundingRect().x();
		float y = boundingRect().y();
		float w = boundingRect().width();
		float h = boundingRect().height();
		//        std::cout << "Delta(): " << deltaX << ", " << deltaY << std::endl;
		//  std::cout << "newRect: " << x << ", " << y << " (" << w << " x " << h << ")" << std::endl;
		QRectF newRect = boundingRect();
		// Move the upper left corner as it is grown
		if (m_CurrentResizeHandle == UserInitArea::UPPER_LEFT_CTRL_POINT)
		{
			newRect.setX(x + deltaX);
			newRect.setY(y + deltaY);
			newRect.setWidth(w - deltaX);
			newRect.setHeight(h - deltaY);
		}
		else if (m_CurrentResizeHandle == UserInitArea::UPPER_RIGHT_CTRL_POINT)
		{
			newRect.setY(y + deltaY);
			newRect.setWidth(w + deltaX);
		}
		else if (m_CurrentResizeHandle == UserInitArea::LOWER_LEFT_CTRL_POINT)
		{
			newRect.setX(x + deltaX);
			newRect.setHeight(h + deltaY);
		}
		else if (m_CurrentResizeHandle == UserInitArea::LOWER_RIGHT_CTRL_POINT)
		{
			newRect.setWidth(w + deltaX);
			newRect.setHeight(h + deltaY);
		}
		prepareGeometryChange();
		setPolygon(QPolygonF(newRect));
	}
	else
	{
		QGraphicsItem::mouseMoveEvent(event);
	}
	emit fireUserInitAreaUpdated(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	UserInitArea::CTRL_POINTS pt = isInResizeArea(event->pos());
	if (m_isResizing || pt != UserInitArea::NO_CTRL_POINT && isSelected() )
	{

		if (pt == UPPER_LEFT_CTRL_POINT || pt == LOWER_RIGHT_CTRL_POINT)
		{
			setCursor(Qt::SizeFDiagCursor);
		}
		else if (pt == UPPER_RIGHT_CTRL_POINT || pt == LOWER_LEFT_CTRL_POINT)
		{
			setCursor(Qt::SizeBDiagCursor);
		}
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}
	QGraphicsItem::hoverMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        deleteSelectedItems(scene());
        break;
    case Qt::Key_Insert:
        duplicateSelectedItems(scene());
        break;
    case Qt::Key_Plus:
        growSelectedItems(scene());
        break;
    case Qt::Key_Minus:
        shrinkSelectedItems(scene());
        break;
    default:
        QGraphicsItem::keyPressEvent(event);
        break;
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea::CTRL_POINTS UserInitArea::isInResizeArea(const QPointF &pos)
{
	float x = boundingRect().x();
	float y = boundingRect().y();
	float w = boundingRect().width();
	float h = boundingRect().height();


	QRectF upLeft(x, y, ctrlPointSize, ctrlPointSize);
	QRectF upRight(x + w - ctrlPointSize, y, ctrlPointSize, ctrlPointSize);
	QRectF lowRight(x + w - ctrlPointSize, y + h - ctrlPointSize, ctrlPointSize, ctrlPointSize);
	QRectF lowLeft(x, y + h - ctrlPointSize, ctrlPointSize, ctrlPointSize);

	if (upLeft.contains(pos))
	{
		//    std::cout << "UPPER_LEFT_CTRL_POINT" << std::endl;
		return UPPER_LEFT_CTRL_POINT;
	}
	if (upRight.contains(pos))   {
		//    std::cout << "UPPER_RIGHT_CTRL_POINT" << std::endl;
		return UPPER_RIGHT_CTRL_POINT;
	}
	if (lowRight.contains(pos))   {
		//   std::cout << "LOWER_RIGHT_CTRL_POINT" << std::endl;
		return LOWER_RIGHT_CTRL_POINT;
	}
	if (lowLeft.contains(pos))   {
		//    std::cout << "LOWER_LEFT_CTRL_POINT" << std::endl;
		return LOWER_LEFT_CTRL_POINT;
	}
	return NO_CTRL_POINT;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int UserInitArea::type() const
{
    return Type;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitArea::setColor(QColor color)
{
	setBrush(QBrush(color));
	emit fireUserInitAreaUpdated(this);
}
