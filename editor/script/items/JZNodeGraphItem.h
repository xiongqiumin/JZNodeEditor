#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include <QGraphicsProxyWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"

class JZNodeLineItem;
class JZNodeGraphItem : public JZNodeBaseItem
{
public:
    JZNodeGraphItem();
    ~JZNodeGraphItem();

    void init(JZNode *node);

    virtual QRectF boundingRect() const override;
    virtual void updateNode() override;    
    void updateSize();

    void setPinValue(int pin, QString name);

    JZNode *node();
    int pinAt(QPointF pos);        //连接框
    int pinAtInName(QPointF pos);  //包含连接框和名称矩形  
    QRectF pinRect(int pin);
    QRectF pinNameRect(int pin);
    QSize size() const;
    
    QString getTip(QPointF pt);
        
    void setPinRuntimeValue(int pin_id,const QString &value);    
    void clearRuntimeValue();

    void setError(const QString &error);
    void clearError();
    bool isError() const;
    
    void onTimerEvent(int event);
    void clear();

protected:
    enum
    {
        Timer_longPress,
    };

    enum IconType{ Flow, Circle, Square, Grid, RoundSquare, Diamond };        
    struct Block
    {                
        Block();
        ~Block();
        
        int width();        
        int height();
        void clear();
        bool isPin();
                
        int id;
        int pri;
        bool isInput;
        bool isShowValue;        
        IconType iconType;
        QString name;

        QRect iconRect;
        QRect nameRect;
        QRect valueRect; //valueRect 就是 widget 显示范围       

        QGraphicsProxyWidget *proxy;
        QWidget *widget;
    };    
    typedef QSharedPointer<Block> BlockPtr;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void notifyPropChanged(const QByteArray &buffer);    
    
    BlockPtr fromPin(JZNodePin *pin);
    BlockPtr fromWidget(QWidget *widget,bool isInput);

    QList<int> blockList(bool isInput);
    JZNodePin *pin(int pin_id);
    void drawProp(QPainter *painter,int pinId);
    void drawIcon(QPainter *painter, QRectF rect,IconType type, bool filled, QColor color, QColor innerColor);
    void calcGemo(int pin, int x, int y, Block *gemo);
    virtual void updatePin();
    void updateErrorGemo();            

    QSize m_size;
    QString m_title;
    JZNode *m_node;    
    QRectF m_errorRect;    
    QString m_error;
    QMap<int, BlockPtr> m_blocks;
    QMap<int, QString> m_runtimeValue;
    
    int m_widgetIndex;
    int m_downPin;
    int m_longPress;
};

//JZNodeFunctionItem
class JZNodeFunctionItem : public JZNodeGraphItem
{
public:
    JZNodeFunctionItem();
};

#endif
