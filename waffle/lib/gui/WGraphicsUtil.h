#pragma once

#include <QLineF>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QtGlobal>

namespace Wf {

QLineF getArrowPoints(const QPointF& start,
					  const QPointF& end,
					  double size,
					  double arrowAngleDegree = 30.);

// from QGraphicItem::shape() helper (from Qt's source:qgraphicsitem.cpp)
QPainterPath _qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen);

}
