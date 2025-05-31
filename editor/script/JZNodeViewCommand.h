#ifndef JZNODE_VIEW_COMMAND_H_
#define JZNODE_VIEW_COMMAND_H_

#include <QPointF>
#include <QVariant>
#include <QUndoCommand>
#include "JZNode.h"

enum ViewCommand {
    CreateNode,
    RemoveNode,
    MoveNode,    
    CreateLine,
    RemoveLine,
    NodeChange,
    NodePropertyChange,
    CreateGroup,
    RemoveGroup,
    SetGroup,
    AddLocalVariable,
    RemoveLocalVariable,
    ChangeLocalVariable,
};

class JZNodeAbstractView;
class JZNodeViewCommand : public QUndoCommand
{
public:    
    JZNodeViewCommand(JZNodeAbstractView *view,int type);

    virtual void redo() override;
    virtual void undo() override;       
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int command;
    int itemId;
    int pinId;
    QVariant oldValue;
    QVariant newValue;   
    QPointF oldPos;
    QPointF newPos; 

protected:
    JZNodeAbstractView *m_view;
};

class JZNodePinValueChangedCommand : public QUndoCommand
{
public:
    JZNodePinValueChangedCommand(JZNodeAbstractView *view);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int nodeId;
    int pinId;
    QString oldValue;
    QString newValue;    

protected:
    JZNodeAbstractView *m_view;
};

class JZNodeMoveCommand : public QUndoCommand
{
public:
    struct NodePosInfo
    {
        int itemId;
        QPointF oldPos;
        QPointF newPos;
    };

    JZNodeMoveCommand(JZNodeAbstractView *view, int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);    
   
    int command;
    QList<NodePosInfo> nodeList;

protected:
    JZNodeAbstractView *m_view;
};

class JZNodeVariableCommand : public QUndoCommand
{
public:
    JZNodeVariableCommand(JZNodeAbstractView *view, int type);

    virtual void redo() override;
    virtual void undo() override;

    int command;
    JZParamDefine newParam;
    JZParamDefine oldParam;

protected:
    JZNodeAbstractView *m_view;
};

#endif
