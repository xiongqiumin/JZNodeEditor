#include "JZEditorGlobal.h"
#include "mainwindow.h"

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