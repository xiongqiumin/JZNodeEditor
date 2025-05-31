#include <QVector2d>
#include "JZVisionLineItem.h"
#include "JZVisionView.h"
#include "JZVisionNodeItem.h"

static qreal crossProduct(const QVector2D& a, const QVector2D& b) {
    return a.x() * b.y() - a.y() * b.x();
}

//JZVisionLineItem
JZVisionLineItem::JZVisionLineItem(JZNodeGemo from)
    :JZAbstractLineItem(from)
{
}

QRectF JZVisionLineItem::boundingRect() const
{
    return QRectF(m_startPoint,m_endPoint).normalized();
}

QPainterPath JZVisionLineItem::shape() const
{
    QPointF p1 = m_startPoint;
    QPointF p2 = m_endPoint;

    QPainterPathStroker st;
    st.setWidth(10.0);

    QPainterPath path(p1);
    path.lineTo(p2);
    return st.createStroke(path);
}

void JZVisionLineItem::updateNode()
{
    prepareGeometryChange();

    JZVisionView* view = dynamic_cast<JZVisionView*>(this->scene()->views()[0]);
    JZAbstractNodeItem* node_from = view->getNodeItem(m_from.nodeId);
    m_startPoint = node_from->sceneBoundingRect().center();
    
    if (m_to.nodeId != -1)
    {
        JZAbstractNodeItem* node_to = view->getNodeItem(m_to.nodeId);
        m_endPoint = node_to->sceneBoundingRect().center();
    }
}


void JZVisionLineItem::CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2)
{
    /*
    * @brief 求得箭头两点坐标
    */

    double arrowLength = 10;      // 箭头长度，一般固定
    double arrowDegrees = 0.5;    // 箭头角度，一般固定

    // 求 y / x 的反正切值
    double angle = atan2(endY - startY, endX - startX) + 3.1415926;

    // 求得箭头点 1 的坐标
    x1 = endX + arrowLength * cos(angle - arrowDegrees);
    y1 = endY + arrowLength * sin(angle - arrowDegrees);

    // 求得箭头点 2 的坐标
    x2 = endX + arrowLength * cos(angle + arrowDegrees);
    y2 = endY + arrowLength * sin(angle + arrowDegrees);
}

QPointF JZVisionLineItem::calculateIntersection(const QPointF& rayStart, const QPointF& rayEnd, const QRectF& rect) 
{
    // 计算射线的方向向量
    QPointF direction(rayEnd.x() - rayStart.x(), rayEnd.y() - rayStart.y());

    // 存储所有有效的交点
    QVector<QPointF> intersections;

    // 检查射线起点是否在矩形内
    bool isInside = rect.contains(rayStart);

    // 检查射线与矩形四条边的交点
    // 上边 y = rect.top()
    if (direction.y() != 0) {
        double t = (rect.top() - rayStart.y()) / direction.y();
        if (t >= 0 && t <= 1) {
            double x = rayStart.x() + t * direction.x();
            if (x >= rect.left() && x <= rect.right()) {
                intersections.append(QPointF(x, rect.top()));
            }
        }
    }

    // 下边 y = rect.bottom()
    if (direction.y() != 0) {
        double t = (rect.bottom() - rayStart.y()) / direction.y();
        if (t >= 0 && t <= 1) {
            double x = rayStart.x() + t * direction.x();
            if (x >= rect.left() && x <= rect.right()) {
                intersections.append(QPointF(x, rect.bottom()));
            }
        }
    }

    // 左边 x = rect.left()
    if (direction.x() != 0) {
        double t = (rect.left() - rayStart.x()) / direction.x();
        if (t >= 0 && t <= 1) {
            double y = rayStart.y() + t * direction.y();
            if (y >= rect.top() && y <= rect.bottom()) {
                intersections.append(QPointF(rect.left(), y));
            }
        }
    }

    // 右边 x = rect.right()
    if (direction.x() != 0) {
        double t = (rect.right() - rayStart.x()) / direction.x();
        if (t >= 0 && t <= 1) {
            double y = rayStart.y() + t * direction.y();
            if (y >= rect.top() && y <= rect.bottom()) {
                intersections.append(QPointF(rect.right(), y));
            }
        }
    }

    // 如果没有找到交点，返回空点
    if (intersections.isEmpty()) {
        return QPointF();
    }

    // 如果起点在矩形内，找到最远的交点
    if (isInside) {
        QPointF farthestPoint = intersections.first();
        double maxDistance = QLineF(rayStart, farthestPoint).length();

        for (int i = 1; i < intersections.size(); ++i) {
            double distance = QLineF(rayStart, intersections[i]).length();
            if (distance > maxDistance) {
                maxDistance = distance;
                farthestPoint = intersections[i];
            }
        }

        return farthestPoint;
    }

    // 如果起点在矩形外，找到最近的交点
    QPointF closestPoint = intersections.first();
    double minDistance = QLineF(rayStart, closestPoint).length();

    for (int i = 1; i < intersections.size(); ++i) {
        double distance = QLineF(rayStart, intersections[i]).length();
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = intersections[i];
        }
    }

    return closestPoint;
}

void JZVisionLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
{
    QColor c = Qt::black;
    auto pen_style = Qt::SolidLine;
    if (isSelected())
        c = Qt::yellow;
    if (m_to.nodeId == -1)
        pen_style = Qt::DashLine;

    //在源区域内不画
    JZVisionView* view = dynamic_cast<JZVisionView*>(this->scene()->views()[0]);
    JZAbstractNodeItem* node_from = view->getNodeItem(m_from.nodeId);
    QRectF from_rc = node_from->sceneBoundingRect();
    if (from_rc.contains(m_endPoint))
        return;

    QPointF start = calculateIntersection(m_startPoint, m_endPoint, from_rc);
    QPointF end;
    if (m_to.nodeId != -1)
    {
        JZAbstractNodeItem* node_to = view->getNodeItem(m_to.nodeId);
        QRectF to_rc = node_to->sceneBoundingRect();
        end = calculateIntersection(m_startPoint, m_endPoint, to_rc);
    }
    else
    {
        end = m_endPoint;
    }

    painter->setPen(QPen(QBrush(c), 4, pen_style));
    painter->drawLine(start, end);

    int lineHStartPos = start.x(); // 连接线起点水平位置
    int lineVStartPos = start.y(); // 连接线起点垂直位置
    int lineHEndPos = end.x();   // 连接线终点水平位置
    int lineVEndPos = end.y();   // 连接线终点垂直位置    

    // 箭头的两点坐标
    double x1, y1, x2, y2;

    // 求得箭头两点坐标
    CalcVertexes(lineHStartPos, lineVStartPos, lineHEndPos, lineVEndPos, x1, y1, x2, y2);
    painter->drawLine(lineHEndPos, lineVEndPos, x1, y1); // 绘制箭头一半
    painter->drawLine(lineHEndPos, lineVEndPos, x2, y2); // 绘制箭头另一半
}