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

    virtual void addMenuBar(QMenuBar *menubar) override;
    virtual void removeMenuBar(QMenuBar *menubar) override;

    virtual bool isModified() override;

    virtual void undo() override;
    virtual void redo() override;
    virtual void remove() override;
    virtual void cut() override;
    virtual void copy() override;
    virtual void paste() override;
    virtual void selectAll() override;
    
    BreakPointTriggerResult breakPointTrigger();
    void ensureNodeVisible(int nodeId);
    void selectNode(int nodeId);

    void setRunning(bool staus);
    int runtimeNode();
    void setRuntimeNode(int nodeId);

    void updateNode();

protected slots:
    void onActionLayout();
    void onActionFitInView();

protected:
    void init();

    JZNodeView *m_view;    
    JZNodePanel *m_nodePanel;
    JZNodePropertyEditor *m_nodeProp;
    JZNodeCompiler *m_compiler;
    QList<QAction*> m_actionList;
};

#endif
