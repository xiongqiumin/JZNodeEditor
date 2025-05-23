#include "JZScriptEditor.h"

JZScriptEditor::JZScriptEditor()
{    
    m_type = Editor_ui;    
    m_form = nullptr;

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    this->setLayout(l);    
}

JZScriptEditor::~JZScriptEditor()
{        
    close();
}   

void JZScriptEditor::open(JZProjectItem *item)
{    
    JZUiItem *file = (JZUiItem*)item;
    auto designer = JZDesigner::instance()->editor();    
    m_form = designer->open(file); 
    m_fwm = designer->core()->formWindowManager();
    
    auto editor = m_form->editor();
    QUndoStack *stack = editor->commandHistory();
    connect(stack, &QUndoStack::canRedoChanged, this, &JZScriptEditor::redoAvailable);
    connect(stack, &QUndoStack::canUndoChanged, this, &JZScriptEditor::undoAvailable);
    connect(stack, &QUndoStack::cleanChanged, this, &JZScriptEditor::onCleanChanged);
}

void JZScriptEditor::close()
{
    auto designer = JZDesigner::instance()->editor();    
    designer->close(m_form);
    m_form = nullptr;

    if (designer->parent() == this)
    {
        designer->hide();
        designer->setParent(nullptr);                                     
    }    
}

void JZScriptEditor::save()
{
    JZUiItem *file = (JZUiItem*)m_item;
    QString xml = m_form->editor()->contents();    
    file->setXml(xml);
    m_project->saveItem(file);
    m_form->editor()->setDirty(false);    
}

void JZScriptEditor::active()
{    
    if (!m_form)
        return;

    auto designer = JZDesigner::instance()->editor();
    if (designer->parent() != this)
    {        
        designer->setParent(nullptr);                                    
        layout()->addWidget(designer);                
        designer->show();                        
        designer->showForm(m_form);                
    }    
}

bool JZScriptEditor::isModified()
{
    if (!m_form)
        return false;

    return m_form->editor()->isDirty();
}

void JZScriptEditor::undo()
{
    m_fwm->actionUndo()->trigger();
}

void JZScriptEditor::redo()
{
    m_fwm->actionRedo()->trigger();
}

void JZScriptEditor::remove()
{
    m_fwm->actionDelete()->trigger();
}

void JZScriptEditor::cut()
{
    m_fwm->actionCut()->trigger();
}

void JZScriptEditor::copy()
{
    m_fwm->actionCopy()->trigger();
}

void JZScriptEditor::paste()
{
    m_fwm->actionPaste()->trigger();
}

void JZScriptEditor::selectAll()
{
    m_fwm->actionSelectAll()->trigger();
}

void JZScriptEditor::onCleanChanged(bool flag)
{
    emit modifyChanged(!flag);
}