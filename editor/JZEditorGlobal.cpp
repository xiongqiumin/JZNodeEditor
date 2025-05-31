#include "JZEditorGlobal.h"

class EdiorGlobal
{
public:
    EdiorGlobal();

    JZProject* project;
    QMenuBar* menu;
    QToolBar* toolBar;
};

EdiorGlobal::EdiorGlobal()
{
    project = nullptr;
    menu = nullptr;
    toolBar = nullptr;
}

static EdiorGlobal g_editor;

JZProject* editorProject()
{
    return g_editor.project;
}

void setEditorProject(JZProject* project)
{
    g_editor.project = project;
}

QMenuBar* editorMenuBar()
{
    return g_editor.menu;
}

void setEditorMenuBar(QMenuBar* menu)
{
    g_editor.menu = menu;
}

QToolBar* editorToolBar()
{
    return g_editor.toolBar;
}

void setEditorToolBar(QToolBar* bar)
{
    g_editor.toolBar = bar;
}

JZNodeEditorManager *editorManager()
{
    return JZNodeEditorManager::instance();
}

JZScriptEnvironment *editorEnvironment()
{
    return g_editor.project->environment();
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