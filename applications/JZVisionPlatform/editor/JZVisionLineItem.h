#pragma once

#include "JZNodeBaseItem.h"

class JZVisionLineItem : public JZAbstractLineItem
{
public:
    JZVisionLineItem(JZNodeGemo from);    
    
    QRectF boundingRect() const;
    QPainterPath shape() const;

    void updateNode();    

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2);
    QPointF calculateIntersection(const QPointF& rayOrigin, const QPointF& rayDirection, const QRectF& rect);    
};