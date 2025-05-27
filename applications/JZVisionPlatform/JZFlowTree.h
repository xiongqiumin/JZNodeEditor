#ifndef JZ_FLOW_TREE_H_
#define JZ_FLOW_TREE_H_

#include "JZProjectTree.h"

class JZFlowTree : public JZProjectTree
{
    Q_OBJECT

public:
    JZFlowTree();
    ~JZFlowTree();

protected slots:

protected:
    virtual void init() override;
};

#endif
