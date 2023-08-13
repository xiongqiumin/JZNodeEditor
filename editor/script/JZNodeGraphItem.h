#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZNodeLineItem;

class JZNodeGraphItem;
class JZNodeGraphItem : public JZNodeBaseItem
{
public:
    JZNodeGraphItem(JZNode *node);
    ~JZNodeGraphItem();

    virtual QRectF boundingRect() const override;
    virtual void updateNode() override;

    JZNode *node();
    JZNodePin *propAt(QPointF pos);
    QRectF propRect(int prop);
    QSize size() const;
    
    void setError(QString error);
    void clearError();

protected:
    enum IconType{ Flow, Circle, Square, Grid, RoundSquare, Diamond };    
    struct PropGemo
    {
        PropGemo();
        ~PropGemo();
        
        void clear();
        int width();
        void valueRectChanged();
        
        QRectF iconRect;
        QRectF nameRect;
        QRectF valueRect;

        int widgetType;
        QGraphicsProxyWidget *proxy;
        QWidget *widget;       
    };

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void drawProp(QPainter *painter,int propId);
    void drawIcon(QPainter *painter, QRectF rect,IconType type, bool filled, QColor color, QColor innerColor);
    void calcGemo(int prop, int x, int y,PropGemo *gemo);
    void updatePropGemo();
    void updateErrorGemo();
    void setPropValue(int id, const QVariant &value);

    QSize m_size;    
    JZNode *m_node;    
    QMap<int,PropGemo> m_propRects;
    QRectF m_errorRect;    
    QString m_error;

    bool m_pinButtonOn;
    QRectF m_pinButtonRect;
};

#endif
