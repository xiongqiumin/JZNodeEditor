#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include <QGraphicsProxyWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"
#include "JZScriptEnvironment.h"

class JZNodeLineItem;
class JZNodeGraphItem : public JZNodeBaseItem
{
public:
    enum IconType { Flow, Circle, Square, Grid, RoundSquare, Diamond };
    struct Block
    {
        Block(JZNodeGraphItem *item);
        ~Block();

        int width();
        int height();
        void clear();
        void setWidget(QWidget *widget);
        void clearWidget();
        bool isPin();

        int id;
        int pri;
        bool isInput;
        bool isShowName;
        bool isShowValue;
        bool isEditable;
        IconType iconType;
        QString name;

        QRect iconRect;
        QRect nameRect;
        QRect valueRect; //valueRect 就是 widget 显示范围       

        QGraphicsProxyWidget *proxy;
        QWidget *widget;
        JZNodeGraphItem *item;
    };    
    typedef QSharedPointer<Block> BlockPtr;

    JZNodeGraphItem();
    ~JZNodeGraphItem();

    void init(JZNode *node);
    
    virtual QRectF boundingRect() const override;
    virtual void updateNode() override;
    void updateSize();

    void setPinValue(int pin, QString value);
    QString pinValue(int pin);

    virtual void setBlockValue(int pin, QString value);
    virtual QString blockValue(int pin);

    JZNode *node();
    int pinAt(QPointF pos);        //连接框
    int pinAtInName(QPointF pos);  //包含连接框和名称矩形  
    int pinAtInValueRect(QPointF pos);    
    QRectF pinRect(int pin);
    QRectF pinNameRect(int pin);
    bool isPinEditable(int pin);
    QSize size() const;
    
    Block *block(int id);
    QList<int> blockList(bool isInput);

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

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void notifyPropChanged(const QByteArray &buffer);    
    
    BlockPtr createPinBlock(JZNodePin *pin);
    BlockPtr createWidgetBlock(QWidget *widget,bool isInput);    

    QByteArray saveNode();    
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

    virtual void updatePin();
};

#endif
