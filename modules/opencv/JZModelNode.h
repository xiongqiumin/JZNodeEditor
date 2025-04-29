#ifndef JZ_MODEL_NODE_H_
#define JZ_MODEL_NODE_H_

#include "JZNode.h"
#include "../JZModuleDefine.h"
#include "JZModelManager.h"

enum ModelNode
{
    Node_ModelInit = Module_ModelNode,
    Node_ModelForward,
};

class JZNodeModelInit : public JZNode
{
public:
    JZNodeModelInit();
    ~JZNodeModelInit();

    void setConfig(JZModelManagerConfig config);
    JZModelManagerConfig config();

    bool compiler(JZNodeCompiler* c, QString& error);
    void saveToStream(QDataStream& s) const;
    void loadFromStream(QDataStream& s);

protected:
    JZModelManagerConfig m_config;
};

class JZNodeModelForward : public JZNode
{
public:
    JZNodeModelForward();
    ~JZNodeModelForward();

    bool compiler(JZNodeCompiler *c, QString &error);
};

#endif