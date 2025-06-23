#pragma once

#include "JZNodeBaseItem.h"

class JZVisionLineItem : public JZAbstractLineItem
{
public:
    JZVisionLineItem(JZNodeGemo from); 
    ~JZVisionLineItem();
    
    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void updateNode();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    QPainterPath linePath() const;

    QPointF drawStartPoint() const;
    QPointF drawEndPoint() const;
    void CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2) const;
    bool m_isParam;    
};