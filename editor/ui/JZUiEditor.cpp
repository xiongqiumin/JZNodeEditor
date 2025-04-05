#include "JZUiEditor.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QUndoStack>
#include "JZProject.h"

JZUiEditor::JZUiEditor()
{    
    m_type = Editor_ui;        

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    this->setLayout(l);    
}

JZUiEditor::~JZUiEditor()
{        
}   

void JZUiEditor::open(JZProjectItem *item)
{    
    JZUiFile *file = (JZUiFile*)item;     
        
    QUndoStack *stack = &m_stack;
    connect(stack, &QUndoStack::canRedoChanged, this, &JZUiEditor::redoAvailable);
    connect(stack, &QUndoStack::canUndoChanged, this, &JZUiEditor::undoAvailable);
    connect(stack, &QUndoStack::cleanChanged, this, &JZUiEditor::onCleanChanged);
}

void JZUiEditor::close()
{    
}

void JZUiEditor::save()
{
    
}

void JZUiEditor::active()
{        
}

bool JZUiEditor::isModified()
{
    return !m_stack.isClean();
}

void JZUiEditor::undo()
{
    
}

void JZUiEditor::redo()
{
    
}

void JZUiEditor::remove()
{
    
}

void JZUiEditor::cut()
{
    
}

void JZUiEditor::copy()
{
    
}

void JZUiEditor::paste()
{
    
}

void JZUiEditor::selectAll()
{
    
}

void JZUiEditor::onCleanChanged(bool flag)
{
    emit modifyChanged(!flag);
}