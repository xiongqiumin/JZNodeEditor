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
    return QRectF(m_startPoint, m_endPoint).normalized();
}

QPainterPath JZNodeLineItem::shape() const
{
    QPointF p1 = m_startPoint;
    QPointF p2 = m_endPoint;

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
    auto from = node_from->mapToScene(node_from->propRect(m_from.propId).center());
    m_startPoint = mapFromScene(from);
    if (m_to.nodeId != -1)
    {
        JZNodeGraphItem *node_to = view->getNodeItem(m_to.nodeId);
        auto to = node_to->mapToScene(node_to->propRect(m_to.propId).center());
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

void JZNodeLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget)
{    
    QColor c = Qt::black;
    auto pen_style = Qt::SolidLine;
    if(isSelected())
        c = Qt::yellow;
    if(m_to.nodeId == -1)
        pen_style = Qt::DashLine;

    painter->setPen(QPen(QBrush(c),1,pen_style));
    painter->drawLine(m_startPoint, m_endPoint);
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
