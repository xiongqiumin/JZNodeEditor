#include <QVector2d>
#include <QGraphicsSceneMouseEvent>
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
    m_isParam = false;
}

JZVisionLineItem::~JZVisionLineItem()
{
}

QRectF JZVisionLineItem::boundingRect() const
{
    if (m_isParam)
        return QRectF();

    auto path = linePath();    
    auto rc = path.boundingRect().normalized();
    if (rc.width() < 8)
    {
        auto ct = rc.center();
        rc.setLeft(ct.x() - 4);
        rc.setRight(ct.x() + 4);
    }
    if (rc.height() < 8)
    {
        auto ct = rc.center();
        rc.setTop(ct.y() - 4);
        rc.setBottom(ct.y() + 4);
    }
    return rc;
}


QPainterPath JZVisionLineItem::linePath() const
{
    QPointF start = drawStartPoint();
    QPointF end = drawEndPoint();

    QPainterPath path(start);
    if (start.x() < end.x())
    {
        path.lineTo(end);
    }
    else
    {
        JZAbstractNodeItem *node_from = editor()->getNodeItem(m_from.nodeId);

        double cx = (start.x() + end.x()) / 2;
        double cy = 0;
        if (m_to.nodeId != -1)
        {
            JZAbstractNodeItem *node_to = editor()->getNodeItem(m_to.nodeId);
            QRectF from_rc = node_from->sceneBoundingRect();
            QRectF to_rc = node_to->sceneBoundingRect();
            if (from_rc.bottom() < to_rc.top())
                cy = (from_rc.bottom() + to_rc.top()) / 2;
            else
                cy = (from_rc.top() + to_rc.bottom()) / 2;
        }
        else
        {
            cy = (start.y() + end.y()) / 2;
        }

        //path.quadTo(start + QPointF(20,0), QPointF(start.x() + 20, center.y()));
        path.lineTo(start + QPointF(20, 0));
        path.lineTo(QPointF(start.x() + 20, cy));

        path.lineTo(QPointF(end.x() - 20, cy));
        
        path.lineTo(QPointF(end.x() - 20, end.y()));
        path.lineTo(QPointF(end.x() - 16, end.y()));
        //path.quadTo(end - QPointF(20, 0),end);
    }        

    return path;
}
QPainterPath JZVisionLineItem::shape() const
{    
    QPainterPathStroker st;
    st.setWidth(10.0);

    QPainterPath path = linePath(); 
    
    return st.createStroke(path);
}

void JZVisionLineItem::updateNode()
{
    prepareGeometryChange();

    auto *view = editor();
    JZAbstractNodeItem *node_from = view->getNodeItem(m_from.nodeId);
    auto pin = node_from->node()->pin(m_from.pinId);
    if (pin->isParam())
    {
        m_isParam = true;
        hide();
        return;
    }

    auto from = node_from->mapToScene(node_from->pinRect(m_from.pinId).center());
    m_startPoint = mapFromScene(from);
    if (m_to.nodeId != -1)
    {
        JZAbstractNodeItem *node_to = view->getNodeItem(m_to.nodeId);
        auto to = node_to->mapToScene(node_to->pinRect(m_to.pinId).center());
        m_endPoint = mapFromScene(to);
    }
}

QPointF JZVisionLineItem::drawStartPoint() const
{
    JZAbstractNodeItem *node = editor()->getNodeItem(m_from.nodeId);
    int x = node->sceneBoundingRect().right();
    return QPointF(x, m_startPoint.y());
}

QPointF JZVisionLineItem::drawEndPoint() const
{
    if (m_to.nodeId == -1)
        return m_endPoint;

    JZAbstractNodeItem *node = editor()->getNodeItem(m_to.nodeId);
    int x = node->sceneBoundingRect().left();
    return QPointF(x, m_endPoint.y());
}

// ---------------------------------------------------------------------------------------------------------------------------------------
void JZVisionLineItem::CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2) const
{
    /*
    * @brief 求得箭头两点坐标
    */

    double arrowLength = 16;      // 箭头长度，一般固定
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

void JZVisionLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{
    QColor line_color = Qt::black;
    auto pen_style = Qt::SolidLine;
    if (isSelected() || m_to.nodeId == -1)
        line_color = QColor(250, 156, 62);
    if (m_to.nodeId == -1)
        pen_style = Qt::DashLine;

    auto start = drawStartPoint();
    auto end = drawEndPoint();

    painter->setPen(QPen(QBrush(line_color), 3, pen_style));
    
    QPainterPath path = linePath();    
    painter->drawPath(path);    

    int lineHStartPos = 0; 
    int lineVStartPos = 0; 
    int lineHEndPos = end.x();   // 连接线终点水平位置
    int lineVEndPos = end.y();   // 连接线终点垂直位置    
    if (start.x() < end.x())
    {
        lineHStartPos = start.x();
        lineVStartPos = start.y(); 
    }
    else
    {
        lineHStartPos = end.x() - 20;
        lineVStartPos = end.y(); 
    }

    // 箭头的两点坐标
    double x1, y1, x2, y2;    

    // 求得箭头两点坐标
    CalcVertexes(lineHStartPos, lineVStartPos, lineHEndPos, lineVEndPos, x1, y1, x2, y2);
    QVector<QPoint> points;
    points << QPoint(x1, y1) << QPoint(lineHEndPos, lineVEndPos) << QPoint(x2, y2);
    painter->setPen(Qt::NoPen);
    painter->setBrush(line_color);
    painter->drawPolygon(points.data(), 3); // 绘制箭头一半    
}

void JZVisionLineItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QPointF pos = mouseEvent->scenePos();
    if ((m_startPoint - pos).manhattanLength() < 20 || (m_endPoint - pos).manhattanLength() < 20) {
        mouseEvent->ignore();
        return;
    }
    JZNodeBaseItem::mousePressEvent(mouseEvent);
}