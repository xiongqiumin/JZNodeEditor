#ifndef JZ_MODEL_NODE_H_
#define JZ_MODEL_NODE_H_

#include "JZNode.h"

enum ModelNode
{
    Node_ModelId = 1400,
    Node_ModelForward,
};

class JZNodeModelForward : public JZNode
{
public:
    JZNodeModelForward();
    ~JZNodeModelForward();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif