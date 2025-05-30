#include "JZVisionCommand.h"
#include "JZVisionView.h"
#include "JZNodeFactory.h"
#include "JZNodeUtils.h"
#include "JZEditorGlobal.h"

JZVisionCommand::JZVisionCommand(JZVisionView* view, int type)
{
	m_command = type;
	m_view = view;
}

JZVisionCommand::~JZVisionCommand()
{
}

int JZVisionCommand::id() const
{
    return -1;
}

void JZVisionCommand::undo()
{
    if (m_command == CreateNode)
    {
        m_view->removeNode(itemId);
    }
    else if (m_command == RemoveNode)
    {
        auto node = editorNodeFactory()->loadNode(oldValue.toByteArray());
        node->setId(itemId);
        auto item = m_view->insertNode(node);
        m_view->setNodePos(itemId, oldPos);
    }
    else if (m_command == NodeChange)
    {
        m_view->setNodeData(itemId, oldValue.toByteArray());
    }
    else if (m_command == CreateLine)
    {
        m_view->removeLine(itemId);
    }
    else if (m_command == RemoveLine)
    {
        auto line = JZNodeUtils::fromBuffer<JZNodeConnect>(oldValue.toByteArray());
        line.id = itemId;
        m_view->insertLine(line);
    }
    else
    {
        Q_ASSERT(0);
    }
}

void JZVisionCommand::redo()
{
    if (m_command == CreateNode)
    {
        auto node = editorNodeFactory()->loadNode(newValue.toByteArray());
        JZVisionNodeItem* item = nullptr;
        if (itemId == -1)
        {
            item = m_view->createNode(node);
            itemId = item->id();
        }
        else
        {
            node->setId(itemId);
            item = m_view->insertNode(node);
        }
        m_view->setNodePos(itemId, newPos);
    }
    else if (m_command == RemoveNode)
    {
        m_view->removeNode(itemId);
    }
    else if (m_command == NodeChange)
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
    else if (m_command == CreateLine)
    {
        auto line = JZNodeUtils::fromBuffer<JZNodeConnect>(newValue.toByteArray());
        if (itemId == -1)
        {
            auto item = m_view->createLine(line.from, line.to);
            itemId = item->id();
        }
        else
        {
            line.id = itemId;
            m_view->insertLine(line);
        }
    }
    else if (m_command == RemoveLine)
    {
        m_view->removeLine(itemId);
    }
    else
    {
        Q_ASSERT(0);
    }
}

//JZVisionPinValueChangedCommand
JZVisionPinValueChangedCommand::JZVisionPinValueChangedCommand(JZVisionView* view)
{
    m_view = view;
}

void JZVisionPinValueChangedCommand::redo()
{
    m_view->setNodePinValue(nodeId, pinId, newValue);
}

void JZVisionPinValueChangedCommand::undo()
{
    m_view->setNodePinValue(nodeId, pinId, oldValue);
}

int JZVisionPinValueChangedCommand::id() const
{
    return -1;
}


bool JZVisionPinValueChangedCommand::mergeWith(const QUndoCommand* command)
{
    return false;
}

//JZVisionMoveCommand
JZVisionMoveCommand::JZVisionMoveCommand(JZVisionView* view)
{
    m_view = view;
}

int JZVisionMoveCommand::id() const
{
    return 0;
}

bool JZVisionMoveCommand::mergeWith(const QUndoCommand* cmd)
{
    auto* other = dynamic_cast<const JZVisionMoveCommand*>(cmd);
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

void JZVisionMoveCommand::redo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto& info = nodeList[i];
        m_view->setNodePos(info.itemId, info.newPos);
    }
}

void JZVisionMoveCommand::undo()
{
    for (int i = 0; i < nodeList.size(); i++)
    {
        auto& info = nodeList[i];
        m_view->setNodePos(info.itemId, info.oldPos);
    }
}