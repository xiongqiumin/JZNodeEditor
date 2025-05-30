#include <QVector2d>
#include "JZVisionLineItem.h"
#include "JZVisionView.h"
#include "JZVisionNodeItem.h"

static qreal crossProduct(const QVector2D& a, const QVector2D& b) {
    return a.x() * b.y() - a.y() * b.x();
}

//JZVisionLineItem
JZVisionLineItem::JZVisionLineItem(int from)
{
    m_from = from;
    m_to = -1;
}

QRectF JZVisionLineItem::boundingRect() const
{
    return QRectF();
}

int JZVisionLineItem::startTraget()
{
    return m_from;
}

int JZVisionLineItem::endTraget()
{
    return m_to;
}

void JZVisionLineItem::setEndPoint(QPointF point)
{
    prepareGeometryChange();
    m_endPoint = point;
}

void JZVisionLineItem::setEndTraget(int to)
{
    prepareGeometryChange();
    m_to = to;
}

void JZVisionLineItem::updateNode()
{
    prepareGeometryChange();

    JZVisionView* view = dynamic_cast<JZVisionView*>(this->scene()->views()[0]);
    JZVisionNodeItem* node_from = view->getNodeItem(m_from);

    QRectF from_rc = node_from->sceneBoundingRect();
    QPointF from = from_rc.center();
    if (m_to != -1)
    {
        JZVisionNodeItem* node_to = view->getNodeItem(m_to);
        QRectF to_rc = node_to->sceneBoundingRect();

        QPointF to = calculateIntersection(to, from, to_rc);
        m_endPoint = to;
    }
    
    QPointF p1 = calculateIntersection(from, m_endPoint, from_rc);
    m_startPoint = p1;
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

QPointF JZVisionLineItem::calculateIntersection(const QPointF& rayStart, const QPointF& rayEnd, const QRectF& rect) {
    QPointF closestIntersection;
    qreal minDistance = std::numeric_limits<qreal>::max();
    bool hasIntersection = false;

    // 计算射线方向向量并归一化
    QVector2D direction(rayEnd - rayStart);
    if (direction.isNull())
        return QPointF(); // 起点和终点相同，无法构成射线

    direction.normalize();

    // 矩形的四条边
    QLineF edges[4] = {
        QLineF(rect.topLeft(), rect.topRight()),    // 上边
        QLineF(rect.topRight(), rect.bottomRight()), // 右边
        QLineF(rect.bottomRight(), rect.bottomLeft()), // 下边
        QLineF(rect.bottomLeft(), rect.topLeft())    // 左边
    };

    // 射线方程: P = rayStart + t * direction (t >= 0)
    for (int i = 0; i < 4; ++i) {
        const QLineF& edge = edges[i];

        // 计算线段的向量
        QVector2D edgeVector(edge.p2() - edge.p1());

        // 计算叉积
        qreal denominator = crossProduct(direction, edgeVector);

        // 如果叉积为0，表示射线与边平行或共线
        if (qFuzzyCompare(denominator, 0.0))
            continue;

        // 计算从线段起点到射线起点的向量
        QVector2D s(rayStart - edge.p1());

        // 计算参数t和u
        qreal t = crossProduct(s, edgeVector) / denominator;
        qreal u = crossProduct(s, direction) / denominator;

        // 如果t >= 0且u在[0,1]范围内，则存在交点
        if (t >= 0 && u >= 0 && u <= 1) {
            QPointF intersection = rayStart + t * direction.toPointF();
            qreal distance = QVector2D(intersection - rayStart).length();

            if (distance < minDistance) {
                minDistance = distance;
                closestIntersection = intersection;
                hasIntersection = true;
            }
        }
    }

    return hasIntersection ? closestIntersection : QPointF(); // 返回最近的交点，若无交点则返回空点
}


void JZVisionLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
{
    QColor c = Qt::black;
    auto pen_style = Qt::SolidLine;
    if (isSelected())
        c = Qt::yellow;
    if (m_to == -1)
        pen_style = Qt::DashLine;

    auto start = m_startPoint;
    auto end = m_endPoint;

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