#ifndef JZNODE_GROUP_ITEM_H_
#define JZNODE_GROUP_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZNodeGroupItem : public JZNodeBaseItem
{
public:
    JZNodeGroupItem(int id);
    ~JZNodeGroupItem();
    
    virtual QRectF boundingRect() const;
    virtual void updateNode();

protected:    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;    

    QRectF m_rect;
    QGraphicsTextItem *m_text;
};

#endif
