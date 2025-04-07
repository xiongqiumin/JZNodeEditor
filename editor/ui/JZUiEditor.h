#ifndef JZUI_EDITOR_H_
#define JZUI_EDITOR_H_

#include <QUndoStack>
#include <QScrollArea>
#include "JZEditor.h"
#include "JZUiFile.h"
#include "formresizer.h"

class JZUiEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZUiEditor();
    ~JZUiEditor();       

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;    
    virtual void active() override;

    virtual bool isModified();
    virtual void undo();
    virtual void redo();
    virtual void remove();
    virtual void cut();
    virtual void copy();
    virtual void paste();
    virtual void selectAll();

protected slots:
    void onCleanChanged(bool flag);

protected:            
    QUndoStack m_stack;
    QScrollArea *m_area;
};

#endif
