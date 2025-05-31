#ifndef JZNODE_BASE_ITEM_H_
#define JZNODE_BASE_ITEM_H_

#include <QGraphicsItem>
#include "JZNode.h"

enum
{
    Item_none = QGraphicsItem::UserType,
    Item_line,
    Item_node,
    Item_group,
};

class JZNodeAbstractView;
class JZNodeBaseItem : public QGraphicsItem
{
public:
    JZNodeBaseItem();
    ~JZNodeBaseItem();

    void setBaseZValue(int value);

    int id() const;
    void setId(int id);

    virtual void updateNode() = 0;
    virtual int type() const override;
    JZNodeAbstractView *editor() const;

protected:    
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    int m_id;
    int m_type;
};

class JZAbstractLineItem : public JZNodeBaseItem
{
public:
    JZAbstractLineItem(JZNodeGemo from);
    ~JZAbstractLineItem();

    JZNodeGemo startTraget();
    JZNodeGemo endTraget();

    void setEndPoint(QPointF point);
    void setEndTraget(JZNodeGemo to);

protected:
    JZNodeGemo m_from;
    JZNodeGemo m_to;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

class JZAbstractNodeItem : public JZNodeBaseItem
{
public:
    JZAbstractNodeItem(JZNode *node);
    ~JZAbstractNodeItem();

    virtual QRectF boundingRect() const override;

    JZNode* node();
    QSize size() const;
    void setBaseZValue(int value);

    virtual int pinAt(QPointF pos) = 0;
    virtual QRectF pinRect(int pin) = 0;

    bool isError();
    virtual void clearError();
    virtual void setError(QString error);
    virtual QString getTip(QPointF pt);

protected:
    int m_baseZValue;
    
    JZNode *m_node;    
    QSize m_size;
    QString m_error;
};


#endif
