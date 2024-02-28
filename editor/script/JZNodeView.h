﻿#ifndef JZNODE_VIEW_H_
#define JZNODE_VIEW_H_

#include <QGraphicsView>
#include <functional>
#include <QMap>
#include <QTextEdit>
#include "JZNodeScene.h"
#include "JZNodeGraphItem.h"
#include "JZNodeLineItem.h"
#include "JZNodeGroupItem.h"
#include "JZNodePropertyEditor.h"
#include "JZScriptItem.h"
#include <QShortcut>
#include <QUndoStack>
#include <QGraphicsRectItem>
#include "JZNodeProgram.h"
#include "JZNodeViewMap.h"

class JZNodeView;
class JZProject;
class JZNodeViewCommand : public QUndoCommand
{
public:
    enum{
        CreateNode,
        RemoveNode,
        MoveNode,
        NodePropertyChange,
        CreateLine,
        RemoveLine,
        CreateGroup,
        RemoveGroup,
        SetGroup,
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
    JZNodeView *m_view;
};


class BreakPointTriggerResult
{
public:
    enum{
        none,
        add,
        remove,
    };

    BreakPointTriggerResult();

    int type;
    QString filename;
    int nodeId;
};

class JZNodeView : public QGraphicsView
{
    Q_OBJECT

public:
    JZNodeView(QWidget *widget = nullptr);
    virtual ~JZNodeView();

    void setPropertyEditor(JZNodePropertyEditor *propEditor);
    
    void setFile(JZScriptItem *file);
    JZScriptItem *file();

    bool isModified();

    /* node */
    JZNode *getNode(int id);
    JZNodePin *getPin(JZNodeGemo gemo);
    JZNodeGraphItem *createNode(JZNodePtr node);
    JZNodeGraphItem *insertNode(JZNodePtr node);    
    void removeNode(int id);   //只remove node,需要在remove node之前先删除所有连线。

    QByteArray getNodeData(int id);
    void setNodeData(int id,const QByteArray &buffer);
    void setNodePos(int id, QPointF pos);

    void pinClicked(int nodeId,int pinId);
    void updateNode(int id);
    bool isPropEditable(int id,int propId);

    JZNodeGraphItem *createNodeItem(int id);    
    JZNodeGraphItem *getNodeItem(int id);

    void setNodePropValue(int nodeId, int prop_id,QString value);
    QString getNodePropValue(int nodeId, int prop_id);
    void longPressCheck(int nodeId);

    /* connect */
    JZNodeLineItem *createLine(JZNodeGemo from, JZNodeGemo to);
    JZNodeLineItem *insertLine(const JZNodeConnect &connect);
    void removeLine(int id);    

    JZNodeLineItem *createLineItem(int id);    
    JZNodeLineItem *getLineItem(int id);    
    void startLine(JZNodeGemo from);
    void endLine(JZNodeGemo to);
    void cancelLine();
    
    /* group */
    JZNodeGroupItem *createGroup(const JZNodeGroup &group);
    JZNodeGroupItem *insertGroup(const JZNodeGroup &group);
    void removeGroup(int id);
    JZNodeGroupItem *createGroupItem(int id);
    JZNodeGroupItem *getGroupItem(int id);
    QByteArray getGroupData(int id);
    void setGroupData(int id, QByteArray buffer);
    void updateGroup(int id);

    void addTip(QRectF tipArea,QString tip);
    void clearTip();

    QVariant onItemChange(JZNodeBaseItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value);

    void clear();

    void save();
    void redo();
    void undo();
    void remove();
    void cut();
    void copy();
    void paste();
    void selectAll();

    void updateNodeLayout();    
    void fitNodeView();
    void ensureNodeVisible(int id);
    void selectNode(int id);
    BreakPointTriggerResult breakPointTrigger();

    void setRunning(bool flag);

    int runtimeNode();
    void setRuntimeNode(int nodeId);
    bool isBreakPoint(int nodeId);

signals:
    void redoAvailable(bool available);
    void undoAvailable(bool available);
    void modifyChanged(bool modify);    
    void sigFunctionOpen(QString name);

protected slots:
    void onContextMenu(const QPoint &pos);
    void onItemPropChanged();
    void onPropNameUpdate(int nodeId,int propId,const QString &value);
    void onPropUpdate(int nodeId,int propId,const QString &value);
    void onAutoCompiler();
    void onCleanChanged(bool modify);
    void onUndoStackChanged();
    void onMapSceneChanged(QRectF rc);
    void onMapSceneScaled(bool flag);

protected:
    friend JZNodeViewCommand;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;    

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    virtual void drawBackground(QPainter* painter, const QRectF& r) override;

    JZNodeGraphItem *nodeItemAt(QPoint pos);
    void foreachNode(std::function<void(JZNodeGraphItem *)> func, int nodeType = -1);
    void foreachLine(std::function<void(JZNodeLineItem *)> func);    
    void copyItems(QList<QGraphicsItem*> item);
    void removeItems(QList<QGraphicsItem*> item);
    void removeItem(QGraphicsItem *item);
    QList<JZNodeGraphItem*> selectNodeItems();
    void initGraph();            
    void setSelectNode(int id);
    void updatePropEditable(const JZNodeGemo &gemo);
    void saveNodePos();
    void sceneScale(QPoint center, bool up);

    void addCreateNodeCommand(const QByteArray &buffer,QPointF pt);
    void addPropChangedCommand(int id,const QByteArray &oldValue);
    void addMoveNodeCommand(int id, QPointF pt);
    
    void addRemoveLineCommand(int line_id);
    
    int addCreateGroupCommand(const JZNodeGroup &group);
    void addRemoveGroupCommand(int id);
    void addSetGroupCommand(int id, const JZNodeGroup &group);

    int getVariableType(const QString &param_name);        
    void autoCompiler();
    QString getExpr();
    int popMenu(QStringList list);
    QStringList matchParmas(JZNodeObjectDefine *define,int type,QString pre);

    JZNodeViewMap *m_map;
    JZNodeScene *m_scene;
    JZScriptItem *m_file;    
    JZNodeLineItem *m_selLine;    
    QGraphicsTextItem *m_tip;
    QRectF m_tipArea;

    bool m_loadFlag;
    JZNodePropertyEditor *m_propEditor;
    QUndoStack m_commandStack;        
    bool m_recordMove;
    bool m_groupIsMoving;
    bool m_propEditFlag;

    QPointF m_downPoint;
    QTimer *m_compilerTimer;      

    bool m_runningMode;
    int m_runNode;
};

#endif
