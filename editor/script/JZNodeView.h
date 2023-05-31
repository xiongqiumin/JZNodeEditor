#ifndef JZNODE_VIEW_H_
#define JZNODE_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include "JZNodeGraphItem.h"
#include "JZNodeScene.h"
#include "JZNodeLineItem.h"
#include "JZNodePropertyEditor.h"
#include "JZScriptFile.h"
#include <QShortcut>
#include <QUndoStack>
#include <QGraphicsRectItem>

class JZNodeView;
class JZNodeViewCommand : public QUndoCommand
{
public:
    enum{
        CreateNode,
        RemoveNode,
        MoveNode,
        CreateLine,
        RemoveLine,
        PropertyChange,
    };

    JZNodeViewCommand(JZNodeView *view,int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int command;
    int itemId;
    QVariant oldValue;
    QVariant newValue;   
    QPointF oldPos;
    QPointF newPos; 

protected:
    void setItemPos(JZNodeGraphItem *item,QPointF pos);
    QVariant saveItem(JZNodeGraphItem *item);
    void loadItem(JZNodeGraphItem *item,const QVariant &value);

    JZNodeView *m_view;
};

class JZNodeView : public QGraphicsView
{
    Q_OBJECT

public:
    JZNodeView(QWidget *widget = nullptr);
    virtual ~JZNodeView();

    void setPropertyEditor(JZNodePropertyEditor *propEditor);
    void setFile(JZScriptFile *file);

    /* node */
    JZNode *getNode(int id);
    JZNodePin *getPin(JZNodeGemo gemo);
    JZNodeGraphItem *createNode(JZNodePtr node);
    JZNodeGraphItem *insertNode(JZNodePtr node);
    void moveNode(int id,QPointF pos);
    void removeNode(int id);    
    void addPin(int id,JZNodePin pin);
    void removePin(int id,int prop_id);

    JZNodeGraphItem *createNodeItem(int id);    
    JZNodeGraphItem *getNodeItem(int id);

    /* connect */
    JZNodeLineItem *createLine(JZNodeGemo from, JZNodeGemo to);
    JZNodeLineItem *insertLine(const JZNodeConnect &connect);
    void removeLine(int id);    

    JZNodeLineItem *createLineItem(int id);    
    JZNodeLineItem *getLineItem(int id);
    void startLine(JZNodeGemo from);
    void endLine(JZNodeGemo to);
    void cancelLine();

    QVariant onItemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value);

    void clear();
    void redo();
    void undo();
    void remove();
    void cut();
    void copy();
    void paste();
    void selectAll();

    void updateNodeLayout();    
    void setMoveUndo(bool flag);

signals:
    void redoAvailable(bool available);
    void undoAvailable(bool available);

protected slots:
    void onContextMenu(const QPoint &pos);
    void onPropUpdate(int nodeId);
    void onTimer();

protected:      
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void keyReleaseEvent(QKeyEvent *event) override;

    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    virtual void drawBackground(QPainter* painter, const QRectF& r) override;

    void foreachNode(std::function<void(JZNodeGraphItem *)> func, int nodeType = -1);
    void foreachLine(std::function<void(JZNodeLineItem *)> func);    
    void copyItems(QList<QGraphicsItem*> item);
    void removeItems(QList<QGraphicsItem*> item);
    void removeItem(QGraphicsItem *item);    
    void initGraph();
    bool canConnect(JZNodeGemo from,JZNodeGemo to);
    void cancelSelect();

    JZNodeScene *m_scene;
    JZScriptFile *m_file;
    JZNodeLineItem *m_selLine;
    QGraphicsRectItem *m_selArea;
    bool m_loadFlag;
    JZNodePropertyEditor *m_propEditor;
    QUndoStack m_commandStack;        
    bool m_moveUndo;

    QPoint m_downPoint;    
    bool m_isMove;              
};

#endif
