#include <QPainter>
#include "JZVisionItemBase.h"
#include "JZVisionView.h"

JZVisionItemBase::JZVisionItemBase()
{
    m_type = VisionItem_none;
    m_id = INVALID_ID;
}

JZVisionItemBase::~JZVisionItemBase()
{
}

int JZVisionItemBase::id() const
{
    return m_id;
}

void JZVisionItemBase::setId(int id)
{
    m_id = id;
}

JZVisionView* JZVisionItemBase::editor() const
{
    if (this->scene() && this->scene()->views().size() > 0)
        return qobject_cast<JZVisionView*>(this->scene()->views()[0]);
    else
        return nullptr;
}

int JZVisionItemBase::type() const
{
    return m_type;
}

QVariant JZVisionItemBase::itemChange(GraphicsItemChange change, const QVariant& value)
{
    auto edit = editor();
    if (!edit)
        return QGraphicsItem::itemChange(change, value);

    return edit->onItemChange(this, change, value);
}
