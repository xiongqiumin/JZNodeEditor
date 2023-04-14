#ifndef JZNODE_LINE_H_
#define JZNODE_LINE_H_

#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZNodeLineItem : public JZNodeBaseItem
{
public:
    JZNodeLineItem(JZNodeGemo from);
    ~JZNodeLineItem();

    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void updateNode();    

    JZNodeGemo startTraget();
    JZNodeGemo endTraget();

    void setEndPoint(QPointF point);
    void setEndTraget(JZNodeGemo to);
    void setDrag(bool flag);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget);    

    JZNodeGemo m_from;
    JZNodeGemo m_to;
    bool m_drag;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

#endif
