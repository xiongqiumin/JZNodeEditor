#include "JZUiEditor.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QUndoStack>
#include <QPushButton>
#include "JZProject.h"

JZUiEditor::JZUiEditor()
{    
    m_type = Editor_ui;        

    QWidget *widget = new QWidget();
    QPushButton *btn = new QPushButton(widget);
    btn->setGeometry(50, 50,100,50);

    FormResizer *form = new FormResizer();    
    form->setFormWindow(widget);
    widget->resize(400, 200);

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);        
    m_area = new QScrollArea();
    m_area->setWidget(form);

    l->addWidget(m_area);
    this->setLayout(l);
}

JZUiEditor::~JZUiEditor()
{        
}   

void JZUiEditor::open(JZProjectItem *item)
{    
    JZUiItem *file = (JZUiItem*)item;     
        
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