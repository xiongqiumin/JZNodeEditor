#pragma once
#include <QGraphicsItem>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZVisionNodeItem : public JZAbstractNodeItem
{

public:
    JZVisionNodeItem(JZNode *node);    

    void updateNode();
    virtual int pinAt(QPointF pos);
    virtual QRectF pinRect(int pin);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    int m_downPin;
    QPointF m_downPoint;
};