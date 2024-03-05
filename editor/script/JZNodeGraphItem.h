#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"
#include "JZNodeParamWidget.h"

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
    int propAt(QPointF pos);
    QRectF propRect(int pin);
    QSize size() const;

    void setPinValue(int prop_id,const QString &value);
    QString pinValue(int prop_id);
    void resetPropValue();

    void setError(const QString &error);
    void clearError();
    bool isError() const;

    void onTimerEvent(int event);

protected:
    enum
    {
        Timer_toolTip,
        Timer_longPress,
    };

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
        JZNodeParamValueWidget *widget;
    };

    struct TipInfo
    {
        TipInfo();


        QPointF pos;
        QString tips;
    };

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void drawProp(QPainter *painter,int pinId);
    void drawIcon(QPainter *painter, QRectF rect,IconType type, bool filled, QColor color, QColor innerColor);
    void calcGemo(int pin, int x, int y,PropGemo *gemo);
    void updatePropGemo();
    void updateErrorGemo();
    void showTip(QPointF pt,QString text);
    void clearTip();
    
    void setWidgetValue(int prop_id, const QString &value);
    QString getWidgetValue(int prop_id);

    QSize m_size;    
    JZNode *m_node;    
    QMap<int,PropGemo> m_pinRects;
    QRectF m_errorRect;    
    QString m_error;

    bool m_pinButtonOn;
    int m_longPress;
    QRectF m_pinButtonRect;
    TipInfo m_tip;
};

#endif
