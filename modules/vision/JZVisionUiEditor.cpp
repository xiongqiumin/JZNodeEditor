#include "JZVisionUiEditor.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QUndoStack>
#include "JZProject.h"

JZVisionUiEditor::JZVisionUiEditor()
{    
    m_type = Editor_visionUi;    

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    this->setLayout(l);    
}

JZVisionUiEditor::~JZVisionUiEditor()
{        
    close();
}   

void JZVisionUiEditor::open(JZProjectItem *item)
{        
}

void JZVisionUiEditor::close()
{    
}

void JZVisionUiEditor::save()
{    
}

void JZVisionUiEditor::active()
{        
}