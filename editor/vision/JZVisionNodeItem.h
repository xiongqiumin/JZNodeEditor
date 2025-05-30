#pragma once
#include <QGraphicsItem>
#include "JZVisionItemBase.h"

class JZVisionNodeItem : public JZVisionItemBase
{

public:
    JZVisionNodeItem();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void updateNode();
protected:
};