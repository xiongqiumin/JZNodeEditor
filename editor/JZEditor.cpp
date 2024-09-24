#include <QUrl>
#include "JZEditor.h"

JZEditor::JZEditor()
{
    m_item = nullptr;
    m_project = nullptr;
    m_type = Editor_none;
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