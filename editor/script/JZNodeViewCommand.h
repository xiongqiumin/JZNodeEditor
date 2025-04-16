#ifndef JZNODE_VIEW_COMMAND_H_
#define JZNODE_VIEW_COMMAND_H_

#include <QPointF>
#include <QVariant>
#include <QUndoCommand>
#include "JZNode.h"

JZNodeConnect parseLine(const QByteArray &buffer);
QByteArray formatLine(const JZNodeConnect &line);
JZNodeGroup parseGroup(const QByteArray &buffer);
QByteArray formatGroup(const JZNodeGroup &group);

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

class JZNodeView;
class JZNodeViewCommand : public QUndoCommand
{
public:    
    JZNodeViewCommand(JZNodeView *view,int type);

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
    JZNodeView *m_view;
};

class JZNodePinValueChangedCommand : public QUndoCommand
{
public:
    JZNodePinValueChangedCommand(JZNodeView *view);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);

    int nodeId;
    int pinId;
    QString oldValue;
    QString newValue;    

protected:
    JZNodeView *m_view;
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

    JZNodeMoveCommand(JZNodeView *view, int type);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand *command);    
   
    int command;
    QList<NodePosInfo> nodeList;

protected:
    JZNodeView *m_view;
};

class JZNodeVariableCommand : public QUndoCommand
{
public:
    JZNodeVariableCommand(JZNodeView *view, int type);

    virtual void redo() override;
    virtual void undo() override;

    int command;
    JZParamDefine newParam;
    JZParamDefine oldParam;

protected:
    JZNodeView *m_view;
};

#endif
