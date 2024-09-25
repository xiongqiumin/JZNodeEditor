#include "JZEditorGlobal.h"
#include "mainwindow.h"

static JZScriptEnvironment *g_env = nullptr;
JZCORE_EXPORT void setEditorEnvironment(JZScriptEnvironment *env)
{
    g_env = env;
}

JZScriptEnvironment *editorEnvironment()
{
    if(g_env)
        return g_env;

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