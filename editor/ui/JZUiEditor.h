#ifndef JZUI_EDITOR_H_
#define JZUI_EDITOR_H_

#include "JZEditor.h"
#include "JZUiFile.h"

class JZUiEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZUiEditor();
    ~JZUiEditor();

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;    

protected slots:


protected:
    void init();    
};

#endif
