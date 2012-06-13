#pragma once

#include <QFrame>

class QGraphicsScene;
class QGraphicsView;
class QSlider;
class QSplitter;
class QToolButton;
class WImageViewerGraphicsScene;
class WImageViewerGraphicsView;
class WIntensityProfileWidget;
class WCrossSplitter;

class WImageViewer : public QFrame
{
    Q_OBJECT
public:
    WImageViewer(QWidget* parent=0);
    virtual ~WImageViewer();
signals:

public slots:
	void setImage(const QImage& image);
	void zoomIn(int level=1);
	void zoomOut(int level=1);
	void zoomReset();
protected:
	virtual void mouseMoveEvent(QMouseEvent* e);
private slots:
	void setupMatrix();
private:
	void drawImage(QPainter* painter, const QRectF& rect);
	void drawTextPath(QPainter* painter, const QFont& font, const QRectF& rect, const QString& text);
	void drawOverview(QPainter* painter);
	void updateProfile();
	QString pixelInfo(const QPointF& posF);
	// void showProfile(const QRectF& viewableRect);
	void updateOffset(const QPointF& offset);
	qreal scale() const;

	QImage m_image;
	WImageViewerGraphicsView* m_graphicsView;
	WImageViewerGraphicsScene* m_scene;
	WIntensityProfileWidget* m_horzProfile;
	WIntensityProfileWidget* m_vertProfile;
	QToolButton* m_zoomResetButton;
	QSlider* m_zoomSlider;
	friend class WImageViewerGraphicsScene;
	friend class WImageViewerGraphicsView;
};

