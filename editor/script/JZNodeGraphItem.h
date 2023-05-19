#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZNodeLineItem;

class JZNodeGraphItem;
class DispWidget : public QWidget
{
    Q_OBJECT

public:
    DispWidget();
    ~DispWidget();

    void setItem(JZNodeGraphItem *item);
    JZNodeGraphItem *item();

    void addVariable(const JZNodePin &prop);
    void setVariable(int id, QVariant value);
    QVariant getVariable(int id);
    void clear();    

signals:
    void sigValueChanged(int id, QVariant value);

protected slots:
    void onValueChanged();

protected:
    QMap<int, QWidget*> m_widgets;
    JZNodeGraphItem *m_item;
};

class JZNodeGraphItem : public JZNodeBaseItem
{
public:
    JZNodeGraphItem(JZNode *node);
    ~JZNodeGraphItem();

    virtual QRectF boundingRect() const override;
    virtual void updateNode() override;
    void setValue(int prop,QVariant value);

    JZNode *node();
    JZNodePin *propAt(QPointF pos);
    QRectF propRect(int prop);

    DispWidget *widget();

protected:
    enum IconType{ Flow, Circle, Square, Grid, RoundSquare, Diamond };

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void drawProp(QPainter *painter,int propId);
    void drawIcon(QPainter *painter, QRectF rect,IconType type, bool filled, QColor color, QColor innerColor);

    QSize m_size;    
    JZNode *m_node;
    DispWidget *m_dispWidget;
    QGraphicsProxyWidget *m_proxy;
    QMap<int,QRectF> m_propRects;
};

#endif
