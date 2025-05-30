#pragma once

#include <QUndoCommand>
#include <QVariant>

class JZVisionView;
class JZVisionCommand : public QUndoCommand
{    
public:
    enum
    {
        CreateNode,
        RemoveNode,
        MoveNode,
        CreateLine,
        RemoveLine,
        NodeChange,
    };

    explicit JZVisionCommand(JZVisionView *view, int type);
    ~JZVisionCommand();

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;

    int itemId;
    QVariant oldValue;
    QVariant newValue;
    QPointF oldPos;
    QPointF newPos;

protected:
    int m_command;
    JZVisionView* m_view;
};

//JZVisionMoveCommand
class JZVisionPinValueChangedCommand : public QUndoCommand
{
public:
    JZVisionPinValueChangedCommand(JZVisionView* view);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* command);

    int nodeId;
    int pinId;
    QString oldValue;
    QString newValue;

protected:
    JZVisionView* m_view;
};

//JZVisionMoveCommand
class JZVisionMoveCommand : public QUndoCommand
{
public:
    struct NodePosInfo
    {
        int itemId;
        QPointF oldPos;
        QPointF newPos;
    };

    JZVisionMoveCommand(JZVisionView* view);

    virtual void redo() override;
    virtual void undo() override;
    virtual int id() const override;
    virtual bool mergeWith(const QUndoCommand* command);

    QList<NodePosInfo> nodeList;

protected:
    JZVisionView* m_view;
};