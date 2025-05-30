#pragma once
#include "JZVisionItemBase.h"

class JZVisionLineItem : public JZVisionItemBase
{
public:
    JZVisionLineItem(int from);
    QRectF boundingRect() const override;
    
    void updateNode();

    int startTraget();
    int endTraget();

    void setEndPoint(QPointF point);
    void setEndTraget(int to);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void CalcVertexes(double startX, double startY, double endX, double endY, double& x1, double& y1, double& x2, double& y2);
    QPointF calculateIntersection(const QPointF& rayOrigin, const QPointF& rayDirection, const QRectF& rect);

    QPointF m_startPoint;
    QPointF m_endPoint;
    int m_from;
    int m_to;
};