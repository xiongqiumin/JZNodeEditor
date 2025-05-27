#include "JZEditorGlobal.h"

static JZProject* g_editorProject = nullptr;
static QMenuBar* g_editorMenu = nullptr;

JZProject* editorProject()
{
    return g_editorProject;
}

void setEditorProject(JZProject* project)
{
    g_editorProject = project;
}

QMenuBar* editorMenuBar()
{
    return g_editorMenu;
}

void setEditorMenuBar(QMenuBar* menu)
{
    g_editorMenu = menu;
}

JZNodeEditorManager *editorManager()
{
    return JZNodeEditorManager::instance();
}

JZScriptEnvironment *editorEnvironment()
{
    return g_editorProject->environment();
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
    return editorEnvironment()->nodeFactory();
}