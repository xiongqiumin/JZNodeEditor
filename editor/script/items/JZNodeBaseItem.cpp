#include <QPainter>
#include "JZNodeBaseItem.h"
#include "JZNodeAbstractView.h"

JZNodeBaseItem::JZNodeBaseItem()
{
    m_type = Item_none;
    m_id = INVALID_ID;
}

JZNodeBaseItem::~JZNodeBaseItem()
{
}

int JZNodeBaseItem::id() const
{
    return m_id;
}

void JZNodeBaseItem::setId(int id)
{
    m_id = id;
}

JZNodeAbstractView *JZNodeBaseItem::editor() const
{
    if (this->scene() && this->scene()->views().size() > 0)
        return dynamic_cast<JZNodeAbstractView*>(this->scene()->views()[0]);
    else
        return nullptr;
}

int JZNodeBaseItem::type() const
{
    return m_type;
}

QVariant JZNodeBaseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    auto edit = editor();
    if (!edit)
        return QGraphicsItem::itemChange(change, value);

    return edit->onItemChange(this, change, value);
}

//JZAbstractLineItem
JZAbstractLineItem::JZAbstractLineItem(JZNodeGemo from)
{
    m_type = Item_line;
    m_from = from;
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(-1);
}

JZAbstractLineItem::~JZAbstractLineItem()
{
}

JZNodeGemo JZAbstractLineItem::startTraget()
{
    return m_from;
}

JZNodeGemo JZAbstractLineItem::endTraget()
{
    return m_to;
}

void JZAbstractLineItem::setEndPoint(QPointF point)
{
    prepareGeometryChange();
    m_endPoint = point;
}

void JZAbstractLineItem::setEndTraget(JZNodeGemo to)
{
    prepareGeometryChange();
    m_to = to;
}

//JZAbstractNodeItem
JZAbstractNodeItem::JZAbstractNodeItem(JZNode *node)    
{
    m_type = Item_node;
    m_node = node;
    m_id = node->id();

    setOpacity(0.9);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);    
    m_baseZValue = 0;

    setAcceptHoverEvents(true);
}

JZAbstractNodeItem::~JZAbstractNodeItem()
{
}

QRectF JZAbstractNodeItem::boundingRect() const
{
    QRectF rc(0, 0, m_size.width(), m_size.height());
    return rc;
}

void JZAbstractNodeItem::setBaseZValue(int value)
{
    m_baseZValue = value;
    setZValue(value);
}

JZNode* JZAbstractNodeItem::node()
{
    return m_node;
}

QSize JZAbstractNodeItem::size() const
{
    return m_size;
}

bool JZAbstractNodeItem::isError()
{
    return !m_error.isEmpty();
}

void JZAbstractNodeItem::clearError()
{
    m_error.clear();
    update();
}

void JZAbstractNodeItem::setError(QString error)
{
    m_error = error;
    update();
}