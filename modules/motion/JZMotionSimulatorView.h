#ifndef JZMOTIONSIMULATORVIEW_H
#define JZMOTIONSIMULATORVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPointF>
#include "JZMotionSimulator.h"

class JZMotionSimulatorView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit JZMotionSimulatorView(QWidget* parent = nullptr);
    ~JZMotionSimulatorView();

    void setSimulator(JZMotionSimulator* simulator);
    void resetView();

    void setShowPath(bool show);
    void setToolDiameter(double diameter);

signals:
    void statusMessageChanged(const QString& message);

protected:

private slots:
    void onPositionUpdated(double x, double y, double z);
    void onSimulationFinished();
    void onTimeUpdated(double currentTime, double totalTime);

private:
    void updateToolPosition();
    void updateStatusBar();

    JZMotionSimulator* m_simulator = nullptr;
    QGraphicsScene* m_scene;
    QGraphicsEllipseItem* m_toolItem = nullptr;
    QGraphicsPathItem* m_pathItem = nullptr;
    QPainterPath m_path;

    QPointF m_currentPos;
    double m_currentZ = 0.0;
    double m_scale = 10.0;
    double m_toolDiameter = 5.0;
    bool m_showGrid = true;
    bool m_showAxes = true;
    bool m_showPath = true;
    bool m_simulating = false;
    QPoint m_lastMousePos;

    double m_currentTime = 0.0;
    double m_totalTime = 0.0;
};

#endif // JZMOTIONSIMULATORVIEW_H    