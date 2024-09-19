#include "JZNodeEditorManager.h"
#include "JZNode.h"

JZNodeEditorManager *JZNodeEditorManager::instance()
{
    static JZNodeEditorManager inst;
    return &inst;
}

JZNodeEditorManager::JZNodeEditorManager()
{
}

JZNodeEditorManager::~JZNodeEditorManager()
{
}

void JZNodeEditorManager::registCustomFunctionNode(QString function, int node_type)
{
    Q_ASSERT(!m_functionMap.contains(function));
    m_functionMap[function] = node_type;
}

int JZNodeEditorManager::customFunctionNode(QString function)
{
    return m_functionMap.value(function, Node_none);
}

void JZNodeEditorManager::unregistCustomFunctionNode(QString function)
{
    m_functionMap.remove(function);
}