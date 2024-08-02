#ifndef JZNODE_EDITOR_H_
#define JZNODE_EDITOR_H_

#include "JZEditor.h"
#include "JZNodeView.h"
#include "JZNodeViewPanel.h"
#include "JZNodeCompiler.h"
#include "JZNodeAutoRunWidget.h"
#include "JZNodePropertyEditor.h"
#include "JZNodeEngine.h"

class JZNodePanel;
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
    
    void breakPointTrigger();
    void ensureNodeVisible(int nodeId);
    void selectNode(int nodeId);

    void setRunningMode(ProcessStatus staus);
    int runtimeNode();
    void setRuntimeNode(int nodeId);

    JZScriptItem *script();
    ScriptDepend scriptTestDepend();

    void setCompilerResult(const CompilerResult *info);
    void setAutoRunResult(const UnitTestResult &result);

    void setNodeValue(int nodeId, int prop_id, const QString &value);
    void setRuntimeValue(int nodeId, int prop_id, const JZNodeDebugParamValue &value);
    void resetPropValue();
    void updateNode();

signals:
    void sigFunctionOpen(QString name);
    void sigAutoCompiler();
    void sigAutoRun();
    void sigRuntimeValueChanged(int id,QString value);

protected slots:
    void onActionLayout();
    void onActionFitInView();
    void onAutoRunChecked();    
    void onAutoRuning();
    void onScaleOne();
    
protected:
    void init();
    QWidget *createMidBar();

    JZNodeView *m_view;    
    JZNodePanel *m_nodePanel;
    JZNodeViewPanel *m_nodeViewPanel;
    JZNodePropertyEditor *m_nodeProp;
    JZNodeAutoRunWidget* m_runProp;
    QTabWidget *m_tabLeft;
    QTabWidget *m_tabProp;
    JZNodeCompiler *m_compiler;
    QList<QAction*> m_actionList;    
};

#endif
