#include <QPainter>
#include "JZNodeBaseItem.h"
#include "JZNodeView.h"

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

JZNodeView *JZNodeBaseItem::editor() const
{
    if (this->scene() && this->scene()->views().size() > 0)
        return (JZNodeView *)this->scene()->views()[0];
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

    return edit->itemChange(this, change, value);
}
