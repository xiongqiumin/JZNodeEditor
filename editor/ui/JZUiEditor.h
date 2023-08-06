#ifndef JZUI_EDITOR_H_
#define JZUI_EDITOR_H_

#include "JZEditor.h"
#include "JZUiFile.h"
#include "JZDesinger.h"
#include "JZDesignerEditor.h"

class QDesignerFormWindowManagerInterface;
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
    JZDesignerFormWindow *m_form;   
    QDesignerFormWindowManagerInterface *m_fwm;
};

#endif
