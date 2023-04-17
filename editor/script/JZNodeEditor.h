#ifndef JZNODE_EDITOR_H_
#define JZNODE_EDITOR_H_

#include "JZNodeView.h"
#include "JZNodePanel.h"
#include "JZNodePropertyEditor.h"
#include "JZEditor.h"

class JZNodeEditor : public JZEditor
{
    Q_OBJECT
    
public:
    JZNodeEditor();
    ~JZNodeEditor();

    void open(JZProjectItem *item);

protected slots:
    void onRun();
    void onBreakPoint();
    void onStepOver();
    void onStepIn();

protected:
    void init();    

    JZNodeView *m_view;
    JZNodePanel *m_nodePanel;
    JZNodePropertyEditor *m_nodeProp;
};

#endif
