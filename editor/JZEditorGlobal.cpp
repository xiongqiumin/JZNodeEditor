#include "JZEditorGlobal.h"
#include "mainwindow.h"

JZScriptEnvironment *editorEnvironment()
{
    return g_mainWindow->project()->environment();
}

JZNodeObjectManager *editorObjectManager()
{
    return g_mainWindow->project()->environment()->objectManager();
}

JZNodeFunctionManager *editorFunctionManager()
{
    return g_mainWindow->project()->environment()->functionManager();
}