#pragma once
#include <QGraphicsItem>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZVisionNodeItem : public JZAbstractNodeItem
{

public:
    JZVisionNodeItem(JZNode *node);    

    void setNodeName(QString name);
    void updateNode();
    virtual int pinAt(QPointF pos);
    virtual QRectF pinRect(int pin);    
    virtual QString getTip(QPointF pt);

protected:    
    struct Block
    {
        int pri;
        int id;
        QString name;
        QRect iconRect;
        QRect nameRect;
    };

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void drawBlock(QPainter *painter, const Block &block);
    int indexOfBlock(int block_id);
    
    bool m_hovered;
    int m_downPin;
    QString m_nodeName;
    QPointF m_downPoint;    
    
    QList<Block> m_blockList;
};