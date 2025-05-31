#ifndef JZNODE_ITEM_H_
#define JZNODE_ITEM_H_

#include <QGraphicsItem>
#include <QWidget>
#include <QGraphicsProxyWidget>
#include "JZNodeBaseItem.h"
#include "JZNode.h"
#include "JZScriptEnvironment.h"
#include "JZNodeParamEditWidget.h"

class JZNodeLineItem;
class JZNodeView;
class JZNodeGraphItem : public JZAbstractNodeItem
{
public:
    enum IconType { Flow, Circle, Square, Grid, RoundSquare, Diamond };

    enum {        
        Pri_subFlow = 0,
        Pri_subFlowParam = 1,
        Pri_flow = 2,
        Pri_flowParam = 3,
        Pri_widget = 4,

        Pri_user = 10,
    };

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
        QString runTimeValue;

        IconType iconType;
        QString name;
        JZParamEditInfo edit;        

        QRect iconRect;
        QRect nameRect;
        QRect valueRect; //valueRect 就是 widget 显示范围       

        QGraphicsProxyWidget *proxy;
        QWidget *widget;
        JZNodeGraphItem *item;
    };    
    typedef QSharedPointer<Block> BlockPtr;

    JZNodeGraphItem(JZNode *node);
    ~JZNodeGraphItem();
        
    virtual void updateNode() override;   //虚函数不能放在构造调用
    void updateSize();

    void setPinValue(int pin, QString value);
    QString pinValue(int pin);

    virtual void setBlockValue(int pin, QString value);
    virtual QString blockValue(int pin);    
    
    virtual int pinAt(QPointF pos) override;        //连接框
    virtual QRectF pinRect(int pin) override;

    int pinAtInName(QPointF pos);  //包含连接框和名称矩形  
    int pinAtInValueRect(QPointF pos);        
    QRectF pinNameRect(int pin);
    bool isPinEditable(int pin);    
    
    Block *block(int id);
    QList<int> blockList(bool isInput);

    virtual QString getTip(QPointF pt) override;
        
    void setPinRuntimeValue(int pin_id,const QString &value);    
    void clearRuntimeValue();

    void setError(const QString &error);
    void clearError();
    bool isError() const;
        
    void clear();

protected:    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *style, QWidget *widget) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    JZNodeView *nodeView();
    void notifyPropChanged(const QByteArray &buffer);    
    
    BlockPtr createPinBlock(JZNodePin *pin);
    BlockPtr createWidgetBlock(QWidget *widget,bool isInput);    
    BlockPtr createEditBlock(QString name, JZParamEditInfo info, bool isInput = true);
    BlockPtr createButtonBlock(QString name, std::function<void()> func, bool isInput = true);

    QByteArray saveNode();    
    JZNodePin *pin(int pin_id);
    void drawProp(QPainter *painter,int pinId);
    void drawIcon(QPainter *painter, QRectF rect,IconType type, bool filled, QColor color, QColor innerColor);
    void calcGemo(int pin, int x, int y, Block *gemo);
    virtual void updatePin();
    void updateErrorGemo();   
    
    QString m_title;    
    QRectF m_errorRect;    
    QString m_error;
    QMap<int, BlockPtr> m_blocks;
    QMap<int, QString> m_runtimeValue;
    
    int m_blockExtId;
    int m_downPin;    
};

#endif
