#ifndef JZNODE_EDITOR_H_
#define JZNODE_EDITOR_H_

#include "JZNodeView.h"
#include "JZNodePanel.h"
#include "JZNodePropertyEditor.h"
#include "JZEditor.h"
#include "JZNodeCompiler.h"

class JZNodeEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZNodeEditor();
    ~JZNodeEditor();

    virtual void open(JZProjectItem *item) override;
    virtual void close() override;
    virtual void save() override;

    virtual void updateMenuBar(QMenuBar *menubar);
    virtual bool isModified() override;

    virtual void undo() override;
    virtual void redo() override;
    virtual void remove() override;
    virtual void cut() override;
    virtual void copy() override;
    virtual void paste() override;
    virtual void selectAll() override;

    void updateNodeLayout();
    BreakPointTriggerResult breakPointTrigger();

protected slots:


protected:
    void init();    
    bool isFirstShow(JZScriptFile* file);

    JZNodeView *m_view;    
    JZNodePanel *m_nodePanel;
    JZNodePropertyEditor *m_nodeProp;
    JZNodeCompiler *m_compiler;
};

#endif
