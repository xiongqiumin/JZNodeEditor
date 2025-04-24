#include "JZEditorGlobal.h"
#include "mainwindow.h"

JZNodeEditorManager *editorManager()
{
    return JZNodeEditorManager::instance();
}

JZScriptEnvironment *editorEnvironment()
{
    return g_mainWindow->project()->environment();
}

JZNodeObjectManager *editorObjectManager()
{
    return editorEnvironment()->objectManager();
}

JZNodeFunctionManager *editorFunctionManager()
{
    return editorEnvironment()->functionManager();
}

JZNodeFactory *editorNodeFactory()
{
    return editorEnvironment()->factoryManager();
}