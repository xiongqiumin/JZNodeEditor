#include "JZUiEditor.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QUndoStack>
#include "JZDesinger.h"
#include "JZDesignerFormWindow.h"
#include <QDesignerFormEditorInterface>
#include <QDesignerFormWindowManagerInterface> 
#include "JZProject.h"

JZUiEditor::JZUiEditor()
{    
    m_type = Editor_ui;    
    m_form = nullptr;

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
    auto designer = JZDesigner::instance()->editor();    
    m_form = designer->open(file); 
    m_fwm = designer->core()->formWindowManager();
    
    auto editor = m_form->editor();
    QUndoStack *stack = editor->commandHistory();
    connect(stack, &QUndoStack::canRedoChanged, this, &JZUiEditor::redoAvailable);
    connect(stack, &QUndoStack::canUndoChanged, this, &JZUiEditor::undoAvailable);
    connect(stack, &QUndoStack::cleanChanged, this, &JZUiEditor::onCleanChanged);
}

void JZUiEditor::close()
{
    auto designer = JZDesigner::instance()->editor();    
    designer->close(m_form);

    if (designer->parent() == this)
    {
        designer->hide();
        designer->setParent(nullptr);                                     
    }    
}

void JZUiEditor::save()
{
    JZUiFile *file = (JZUiFile*)m_item;    
    QString xml = m_form->editor()->contents();    
    file->setXml(xml);
    m_form->editor()->setDirty(false);    
}

void JZUiEditor::active()
{    
    auto designer = JZDesigner::instance()->editor();
    if (designer->parent() != this)
    {        
        designer->setParent(nullptr);                                    
        layout()->addWidget(designer);                
        designer->show();                        
        designer->showForm(m_form);                
    }    
}

bool JZUiEditor::isModified()
{
    return m_form->editor()->isDirty();
}

void JZUiEditor::undo()
{
    m_fwm->actionUndo()->trigger();
}

void JZUiEditor::redo()
{
    m_fwm->actionRedo()->trigger();
}

void JZUiEditor::remove()
{
    m_fwm->actionDelete()->trigger();
}

void JZUiEditor::cut()
{
    m_fwm->actionCut()->trigger();
}

void JZUiEditor::copy()
{
    m_fwm->actionCopy()->trigger();
}

void JZUiEditor::paste()
{
    m_fwm->actionPaste()->trigger();
}

void JZUiEditor::selectAll()
{
    m_fwm->actionSelectAll()->trigger();
}

void JZUiEditor::onCleanChanged(bool flag)
{
    emit modifyChanged(!flag);
}