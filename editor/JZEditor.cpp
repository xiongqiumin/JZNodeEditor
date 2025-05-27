#include <QUrl>
#include <QShortcut>
#include "JZEditor.h"
#include "JZNodeEditor.h"
#include "JZNodeParamEditor.h"
#include "JZEditorGlobal.h"

JZEditor::JZEditor()
{
    m_item = nullptr;
    m_project = nullptr;    
    m_type = ProjectItem_none;

    new QShortcut(QKeySequence("Ctrl+Z"), this, this, &JZEditor::undo, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+Y"), this, this, &JZEditor::redo, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+D"), this, this, &JZEditor::remove, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+X"), this, this, &JZEditor::cut, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+C"), this, this, &JZEditor::copy, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+V"), this, this, &JZEditor::paste, Qt::WidgetWithChildrenShortcut);
    new QShortcut(QKeySequence("Ctrl+A"), this, this, &JZEditor::selectAll, Qt::WidgetWithChildrenShortcut);
}

JZEditor::~JZEditor()
{
}

void JZEditor::setMainWindow(MainWindow *window)
{
    m_window = window;
}

QMenu *JZEditor::menu(int type)
{
    auto menu_bar = editorMenuBar();
    auto list = menu_bar->actions();
    for (int i = 0; i < list.size(); i++)
    {
        auto menu = list[i]->menu();
        if (menu->property("JZMenuType").toInt() == type)
            return menu;
    }
    return nullptr;
}

int JZEditor::type()
{
    return m_type;
}

void JZEditor::setProject(JZProject *project)
{
    m_project = project;
}

JZProject *JZEditor::project()
{
    return m_project;
}

void JZEditor::setItem(JZProjectItem *item)
{
    m_item = item;
}

JZProjectItem *JZEditor::item()
{
    return m_item;
}

bool JZEditor::isModified()
{
    return false;
}

void JZEditor::active()
{

}

void JZEditor::inactive‌()
{

}

void JZEditor::navigate(QUrl url)
{

}

void JZEditor::undo()
{

}

void JZEditor::redo()
{

}

void JZEditor::remove()
{

}

void JZEditor::cut()
{

}

void JZEditor::copy()
{

}

void JZEditor::paste()
{

}

void JZEditor::selectAll()
{

}

//JZEditorManager
JZEditorManager *JZEditorManager::instance()
{
    static JZEditorManager inst;
    return &inst;
}

JZEditorManager::JZEditorManager()
{
    registEditor(ProjectItem_scriptItem, CreateEditor<JZNodeEditor>);
    registEditor(ProjectItem_param, CreateEditor<JZNodeParamEditor>);
    //registEditor(ProjectItem_ui, CreateEditor<JZUiEditor>);
}

JZEditorManager::~JZEditorManager()
{
}

void JZEditorManager::registEditor(int project_item_type, CreateJZEditorFunc func)
{
    m_editorFunc[project_item_type] = func;
}

JZEditor *JZEditorManager::createEditor(int project_item_type)
{
    if (!m_editorFunc.contains(project_item_type))
        return nullptr;

    return m_editorFunc[project_item_type]();
}