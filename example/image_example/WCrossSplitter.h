#pragma once

#include <QFrame>

class QSplitter;

class WCrossSplitter : public QFrame
{
    Q_OBJECT
public:
    WCrossSplitter(QWidget* parent=0);
    virtual ~WCrossSplitter();
	void addUpperWidget(QWidget* w);
	void addLowerWidget(QWidget* w);
	void setHorizontalStretchFactor(int index, int factor);
	void setVerticalStretchFactor(int index, int factor);
private slots:
	void horzUpMoved();
	void horzDownMoved();
protected:
	QSplitter* m_vert;
	QSplitter* m_horzUp;
	QSplitter* m_horzDown;

	virtual void showEvent(QShowEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
};
