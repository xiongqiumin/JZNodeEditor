#ifndef JZ_MODEL_NODE_H_
#define JZ_MODEL_NODE_H_

#include "JZNode.h"

enum ModelNode
{
    Node_ModelId = 1400,
    Node_ModelInit,
    Node_ModelSetting,
};

class JZModelInitNode : public JZNode
{
public:
    JZModelInitNode();
    ~JZModelInitNode();

    bool compiler(JZNodeCompiler *c, QString &error);
};


class JZModelSettingNode : public JZNode
{
public:
    JZModelSettingNode();
    ~JZModelSettingNode();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif