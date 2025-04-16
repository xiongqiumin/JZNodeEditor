#include "JZNodeView.h"
#include "JZNodeGraphItem.h"
#include "JZNodeFactory.h"
#include "JZNodeViewCommand.h"

//line
JZNodeConnect parseLine(const QByteArray &buffer)
{
    JZNodeConnect line;
    QDataStream s(buffer);
    s >> line;
    return line;
}

QByteArray formatLine(const JZNodeConnect &line)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << line;
    return buffer;
}

//group
JZNodeGroup parseGroup(const QByteArray &buffer)
{
    JZNodeGroup group;
    QDataStream s(buffer);
    s >> group;
    return group;
}

QByteArray formatGroup(const JZNodeGroup &group)
{
    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << group;
    return buffer;
}

//JZNodeViewCommand
JZNodeViewCommand::JZNodeViewCommand(JZNodeView *view,int type)
{
    itemId = -1;
    pinId = -1;
    command = type;
    m_view = view;
}

int JZNodeViewCommand::id() const
{
    if (command == NodePropertyChange && pinId != -1)
    {
        return 10000 + JZNodeCompiler::paramId(itemId, pinId);
    }

    return -1;
}

bool JZNodeViewCommand::mergeWith(const QUndoCommand *cmd)
{
    auto *other = dynamic_cast<const JZNodeViewCommand*>(cmd);
    Q_ASSERT(other);

    this->newValue = other->newValue;
    return true;
}

void JZNodeViewCommand::undo()
{
    if(command == CreateNode)
    {        
        m_view->removeNode(itemId);
    }
    else if(command == RemoveNode)
    {
        auto node = JZNodeFactory::instance()->loadNode(oldValue.toByteArray());
        node->setId(itemId);
        auto item = m_view->insertNode(node);
        m_view->setNodePos(itemId,oldPos);
    }    
    else if (command == NodePropertyChange)
    {
        m_view->setNodeData(itemId, oldValue.toByteArray());
    }
    else if(command == CreateLine)
    {        
        m_view->removeLine(itemId);
    }
    else if(command == RemoveLine)
    {
        auto line = parseLine(oldValue.toByteArray());
        line.id = itemId;
        m_view->insertLine(line);
    }
    else if (command == CreateGroup)
    {
        m_view->removeGroup(itemId);        
    }
    else if (command == RemoveGroup)
    {
        auto group = parseGroup(oldValue.toByteArray());
        m_view->insertGroup(group);
    }    
}

void JZNodeViewCommand::redo()
{
    if(command == CreateNode)
    {        
        auto node = JZNodeFactory::instance()->loadNode(newValue.toByteArray());
        JZNodeGraphItem *item = nullptr;
        if(itemId == -1)
        {
            item = m_view->createNode(node);
            itemId = item->id();
        }
        else
        {
            node->setId(itemId);
            item = m_view->insertNode(node);
        }
        m_view->setNodePos(itemId,newPos);
    }
    else if(command == RemoveNode)
    {
        m_view->removeNode(itemId);
    }
    else if (command == NodePropertyChange)
    {
        if (newValue.isNull())
        {
            newValue = m_view->getNodeData(itemId);
            m_view->updateNode(itemId);
        }
        else
        {
            m_view->setNodeData(itemId, newValue.toByteArray());
        }
    }
    else if(command == CreateLine)
    {
        auto line = parseLine(newValue.toByteArray());
        if(itemId == -1)
        {
            auto item = m_view->createLine(line.from,line.to);
            itemId = item->id();
        }
        else
        {
            line.id = itemId;
            m_view->insertLine(line);
        }
    }
    else if(command == RemoveLine)
    {
        m_view->removeLine(itemId);
    }
    else if (command == CreateGroup)
    {
        auto group = parseGroup(newValue.toByteArray());
        if (itemId == -1)
        {
            auto item = m_view->createGroup(group);
            itemId = item->id();
        }
        else
        {
            group.id = itemId;
            m_view->insertGroup(group);
        }
    }
    else if (command == RemoveGroup)
    {
        m_view->removeGroup(itemId);
    }
    else if (command == SetGroup)
    {
        if (newValue.isNull())
        {   
            newValue = m_view->getGroupData(itemId);
            m_view->updateGroup(itemId);
        }
        else
        {
            m_view->setGroupData(itemId,newValue.toByteArray());
        }
    }    
}

//JZNodePinValueChangedCommand
JZNodePinValueChangedCommand::JZNodePinValueChangedCommand(JZNodeView *view)
{
    m_view = view;
}

void JZNodePinValueChangedCommand::redo()
{
    m_view->setNodePinValue(nodeId, pinId, newValue);
}

void JZNodePinValueChangedCommand::undo()
{
    m_view->setNodePinValue(nodeId, pinId, oldValue);
}

int JZNodePinValueChangedCommand::id() const
{
    return -1;
}


bool JZNodePinValueChangedCommand::mergeWith(const QUndoCommand *command)
{
    return false;
}

//JZNodeMoveCommand
JZNodeMoveCommand::JZNodeMoveCommand(JZNodeView *view, int type)
{
    m_view = view;
    command = type;
}

int JZNodeMoveCommand::id() const
{
    return command;
}

bool JZNodeMoveCommand::mergeWith(const QUndoCommand *cmd)
{    
    auto *other = dynamic_cast<const JZNodeMoveCommand*>(cmd);
    Q_ASSERT(other);
    for (int i = 0; i < other->nodeList.size(); i++)
    {
        int index = -1;
        for (int j = 0; j < nodeList.size(); j++)
        {
            if (nodeList[j].itemId == other->nodeList[i].itemId)
            {
                index = j;
                break;
            }
        }

        if (index != -1)
            nodeList[index].newPos = other->nodeList[i].newPos;
        else
            nodeList.push_back(other->nodeList[i]);
    }

    return true;
}

void JZNodeMoveCommand::redo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto &info = nodeList[i];
        m_view->setNodePos(info.itemId, info.newPos);
    }
}

void JZNodeMoveCommand::undo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto &info = nodeList[i];
        m_view->setNodePos(info.itemId, info.oldPos);
    }
}

//JZNodeVariableCommand
JZNodeVariableCommand::JZNodeVariableCommand(JZNodeView *view, int type)
{
    m_view = view;
    command = type;
}

void JZNodeVariableCommand::redo()
{
    if (command == AddLocalVariable)
    {
        m_view->addLocalVariable(newParam);
    }
    else if(command == RemoveLocalVariable)
    {        
        m_view->removeLocalVariable(oldParam.name);
    }
    else if (command == ChangeLocalVariable)
    {
        m_view->changeLocalVariable(oldParam.name,newParam);
    }
}

void JZNodeVariableCommand::undo()
{
    if (command == AddLocalVariable)
    {
        m_view->removeLocalVariable(newParam.name);
    }
    else if (command == RemoveLocalVariable)
    {
        m_view->addLocalVariable(oldParam);
    }
    else if (command == ChangeLocalVariable)
    {
        m_view->changeLocalVariable(newParam.name,oldParam);
    }
}