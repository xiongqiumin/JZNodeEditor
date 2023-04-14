#ifndef JZNODE_EDITOR_H_
#define JZNODE_EDITOR_H_

#include "JZNodeView.h"
#include "JZNodePanel.h"
#include "JZNodePropertyEditor.h"

class JZNodeEditor{

public:
    JZNodeEditor();
    ~JZNodeEditor();

    void open(JZProjectItem *item);

protected:
    void init();    

    JZNodeView *m_view;
    JZNodePanel *m_nodePanel;
    JZNodePropertyEditor *m_nodeProp;
};

#endif
