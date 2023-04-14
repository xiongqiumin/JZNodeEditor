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
    QRectF propRect(int prop, int type);
    DispWidget *widget();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    QSize m_size;
    QStringList m_in;
    QStringList m_out;
    JZNode *m_node;
    DispWidget *m_dispWidget;
    QGraphicsProxyWidget *m_proxy;
};

#endif
