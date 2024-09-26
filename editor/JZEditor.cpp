#include <QUrl>
#include <QShortcut>
#include "JZEditor.h"

JZEditor::JZEditor()
{
    m_item = nullptr;
    m_project = nullptr;
    m_type = Editor_none;

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

void JZEditor::addMenuBar(QMenuBar *menubar)
{

}

void JZEditor::removeMenuBar(QMenuBar *menubar)
{

}

void JZEditor::active()
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

JZScriptClassItem *JZEditor::getClassFile(JZProjectItem *item)
{    
    while (item)
    {
        if (item->itemType() == ProjectItem_class)
            return (JZScriptClassItem*)item;

        item = item->parent();
    }
    return nullptr;
}