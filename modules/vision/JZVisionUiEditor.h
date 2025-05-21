#ifndef JZUI_EDITOR_H_
#define JZUI_EDITOR_H_

#include "JZEditor.h"
#include "JZUiItem.h"
#include "JZDesinger.h"
#include "JZDesignerEditor.h"

class QDesignerFormWindowManagerInterface;
class JZVisionUiEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZVisionUiEditor();
    ~JZVisionUiEditor();       

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;    
    virtual void active() override;

protected:        

};

#endif
