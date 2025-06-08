#include "JZMotionSimulatorView.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>
#include <cmath>

JZMotionSimulatorView::JZMotionSimulatorView(QWidget* parent) : QGraphicsView(parent)
{
    // 创建场景
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // 设置视图属性
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    setDragMode(QGraphicsView::ScrollHandDrag);

    // 初始化参数
    m_scale = 10.0; // 10像素 = 1mm
    m_showGrid = true;
    m_showAxes = true;
    m_showPath = true;
    m_toolDiameter = 5.0;

    // 创建刀具图形项
    m_toolItem = new QGraphicsEllipseItem(0, 0, m_toolDiameter, m_toolDiameter);
    m_toolItem->setBrush(QBrush(Qt::red));
    m_toolItem->setPen(QPen(Qt::darkRed, 0.5));
    m_scene->addItem(m_toolItem);

    // 创建路径图形项
    m_pathItem = new QGraphicsPathItem();
    m_pathItem->setPen(QPen(Qt::blue, 1.0));
    m_scene->addItem(m_pathItem);

    // 居中视图
    centerOn(0, 0);
}

JZMotionSimulatorView::~JZMotionSimulatorView()
{
}

void JZMotionSimulatorView::setSimulator(JZMotionSimulator* simulator)
{
    m_simulator = simulator;

    if (simulator) {
        connect(simulator, &JZMotionSimulator::sigPos, this, &JZMotionSimulatorView::onPositionUpdated);
        connect(simulator, &JZMotionSimulator::sigSimulationFinished, this, &JZMotionSimulatorView::onSimulationFinished);
        connect(simulator, &JZMotionSimulator::sigTimeUpdated, this, &JZMotionSimulatorView::onTimeUpdated);

        // 重置视图
        resetView();
    }
}

void JZMotionSimulatorView::resetView()
{
    // 清除路径
    m_path = QPainterPath();
    m_pathItem->setPath(m_path);

    // 重置刀具位置
    m_currentPos = QPointF(0, 0);
    updateToolPosition();

    // 居中视图
    centerOn(0, 0);

    // 更新状态
    m_simulating = false;
    update();
}

void JZMotionSimulatorView::updateToolPosition()
{
    // 调整刀具位置，使其中心点与当前位置对齐
    m_toolItem->setPos(m_currentPos.x() - m_toolDiameter / 2, m_currentPos.y() - m_toolDiameter / 2);

    // 如果正在模拟，更新路径
    if (m_simulating && m_showPath) {
        m_path.lineTo(m_currentPos);
        m_pathItem->setPath(m_path);
    }

    // 确保刀具在视图内可见
    ensureVisible(m_toolItem, 100, 100);
}

void JZMotionSimulatorView::onPositionUpdated(double x, double y, double z)
{
    m_currentPos = QPointF(x, -y); // Y轴在屏幕上是向下的，所以取负值
    m_currentZ = z;
    m_simulating = true;

    updateToolPosition();
    updateStatusBar();
}

void JZMotionSimulatorView::onSimulationFinished()
{
    m_simulating = false;
    updateStatusBar();
}

void JZMotionSimulatorView::onTimeUpdated(double currentTime, double totalTime)
{
    m_currentTime = currentTime;
    m_totalTime = totalTime;
    updateStatusBar();
}

void JZMotionSimulatorView::updateStatusBar()
{
    QString status = QString("当前位置: X=%1, Y=%2, Z=%3  |  模拟时间: %4/%5秒")
        .arg(m_currentPos.x(), 0, 'f', 2)
        .arg(-m_currentPos.y(), 0, 'f', 2)
        .arg(m_currentZ, 0, 'f', 2)
        .arg(m_currentTime, 0, 'f', 1)
        .arg(m_totalTime, 0, 'f', 1);

    emit statusMessageChanged(status);
}

void JZMotionSimulatorView::setShowPath(bool show)
{
    m_showPath = show;
    m_pathItem->setVisible(show);
}

void JZMotionSimulatorView::setToolDiameter(double diameter)
{
    m_toolDiameter = diameter;
    m_toolItem->setRect(0, 0, diameter, diameter);
    updateToolPosition();
}