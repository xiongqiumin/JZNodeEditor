#ifndef JZNODE_EDITOR_H_
#define JZNODE_EDITOR_H_

#include "JZEditor.h"
#include "JZNodeView.h"
#include "JZNodePanel.h"
#include "JZNodeViewPanel.h"
#include "JZNodeCompiler.h"
#include "JZNodeAutoRunWidget.h"
#include "JZNodePropertyEditor.h"

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

    void setNodeValue(int nodeId, int prop_id, const QString &value);
    void updateNode();

signals:
    void sigFunctionOpen(QString name);

protected slots:
    void onActionLayout();
    void onActionFitInView();

    void onAutoRunChecked();    

protected:
    void init();

    JZNodeView *m_view;    
    JZNodePanel *m_nodePanel;
    JZNodeViewPanel *m_nodeViewPanel;
    JZNodePropertyEditor *m_nodeProp;
    JZNodeAutoRunWidget* m_runProp;
    QTabWidget *m_tabProp;
    JZNodeCompiler *m_compiler;
    QList<QAction*> m_actionList;    
};

#endif
