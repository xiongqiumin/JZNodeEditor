#ifndef JZNODE_BASE_ITEM_H_
#define JZNODE_BASE_ITEM_H_

#include <QGraphicsItem>
#include "JZNodeDefine.h"

enum
{
    Item_none = QGraphicsItem::UserType,
    Item_line,
    Item_node,
    Item_group,
};

class JZNodeView;
class JZNodeBaseItem : public QGraphicsItem
{
public:
    JZNodeBaseItem();
    ~JZNodeBaseItem();

    int id() const;
    void setId(int id);

    virtual void updateNode() = 0;
    virtual int type() const override;
    JZNodeView *editor() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override = 0;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    int m_id;
    int m_type;
};

#endif
