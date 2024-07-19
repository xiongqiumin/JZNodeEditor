#include <math.h>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "JZNodeLineItem.h"
#include "JZNodeView.h"

// JZNodeLineItem
JZNodeLineItem::JZNodeLineItem(JZNodeGemo from)
{
    m_type = Item_line;
    m_from = from;
    m_drag = false;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(-1);
}

JZNodeLineItem::~JZNodeLineItem()
{
}

void JZNodeLineItem::setDrag(bool flag)
{
    setZValue(-1);
    prepareGeometryChange();
    m_drag = flag;
}    

QRectF JZNodeLineItem::boundingRect() const
{    
    auto rc = QRectF(m_startPoint, m_endPoint).normalized();
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

QPainterPath JZNodeLineItem::shape() const
{
    QPointF p1 = drawStartPoint();
    QPointF p2 = drawEndPoint();

    QPainterPathStroker st;
    st.setWidth(10.0);

    QPainterPath path(p1);
    path.lineTo(p2);
    return st.createStroke(path);
}

void JZNodeLineItem::updateNode()
{
    prepareGeometryChange();

    JZNodeView *view = editor();
    JZNodeGraphItem *node_from = view->getNodeItem(m_from.nodeId);
    auto from = node_from->mapToScene(node_from->pinRect(m_from.pinId).center());
    m_startPoint = mapFromScene(from);
    if (m_to.nodeId != -1)
    {
        JZNodeGraphItem *node_to = view->getNodeItem(m_to.nodeId);
        auto to = node_to->mapToScene(node_to->pinRect(m_to.pinId).center());
        m_endPoint = mapFromScene(to);
    }
}

JZNodeGemo JZNodeLineItem::startTraget()
{
    return m_from;
}

JZNodeGemo JZNodeLineItem::endTraget()
{
    return m_to;
}

void JZNodeLineItem::setEndPoint(QPointF point)
{
    prepareGeometryChange();
    m_endPoint = point;
}

void JZNodeLineItem::setEndTraget(JZNodeGemo to)
{
    prepareGeometryChange();
    m_to = to;
}

QPointF JZNodeLineItem::drawStartPoint() const
{
    JZNodeGraphItem *node = editor()->getNodeItem(m_from.nodeId);
    int x = node->sceneBoundingRect().right();
    return QPointF(x, m_startPoint.y());
}

QPointF JZNodeLineItem::drawEndPoint() const
{
    if (m_to.nodeId == -1)
        return m_endPoint;

    JZNodeGraphItem *node = editor()->getNodeItem(m_to.nodeId);
    int x = node->sceneBoundingRect().left();
    return QPointF(x, m_endPoint.y());
}

// ---------------------------------------------------------------------------------------------------------------------------------------
void JZNodeLineItem::CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2)
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

void JZNodeLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{    
    QColor c = Qt::black;
    auto pen_style = Qt::SolidLine;
    if(isSelected())
        c = Qt::yellow;
    if(m_to.nodeId == -1)
        pen_style = Qt::DashLine;

    auto start = drawStartPoint();
    auto end = drawEndPoint();

    painter->setPen(QPen(QBrush(c),4,pen_style));
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

void JZNodeLineItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QPointF pos = mouseEvent->scenePos();
    if((m_startPoint - pos).manhattanLength() < 20 || (m_endPoint - pos).manhattanLength() < 20){
        mouseEvent->ignore();
        return;
    }
    JZNodeBaseItem::mousePressEvent(mouseEvent);
}
