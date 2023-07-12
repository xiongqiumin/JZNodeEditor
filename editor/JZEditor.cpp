#include "JZEditor.h"

JZEditor::JZEditor()
{
    m_file = nullptr;
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

bool JZEditor::isModified()
{
    return false;
}

void JZEditor::setFile(JZProjectItem *file)
{
    m_file = file;
}

JZProjectItem *JZEditor::file()
{
    return m_file;
}

QString JZEditor::filePath()
{
    return m_file->itemPath();
}

void JZEditor::updateMenuBar(QMenuBar *menubar)
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
