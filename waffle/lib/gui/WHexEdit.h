#pragma once

#include "gui/QHexEdit/qhexedit.h"

class WHexEdit : public QHexEdit
{
	Q_OBJECT
public:
	WHexEdit(QWidget* parent=0);
	virtual ~WHexEdit();
	virtual QSize minimumSizeHint() const;
};
