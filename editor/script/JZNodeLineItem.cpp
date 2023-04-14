#include <QDebug>
#include "JZNodeLineItem.h"
#include "JZNodeView.h"

// JZNodeLineItem
JZNodeLineItem::JZNodeLineItem(JZNodeGemo from)
{
    m_type = Item_line;
    m_from = from;
    m_drag = false;
    setZValue(1);
}

JZNodeLineItem::~JZNodeLineItem()
{
}

void JZNodeLineItem::setDrag(bool flag)
{
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
    auto from = node_from->mapToScene(node_from->propRect(m_from.propId, Prop_out).center());
    m_startPoint = mapFromScene(from);
    if (m_to.nodeId != -1)
    {
        JZNodeGraphItem *node_to = view->getNodeItem(m_to.nodeId);
        auto to = node_to->mapToScene(node_to->propRect(m_to.propId, Prop_in).center());
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
    painter->drawLine(m_startPoint, m_endPoint);
}
