#ifndef JZ_OPEN_CV_NODE_H_
#define JZ_OPEN_CV_NODE_H_

#include <opencv2/opencv.hpp>
#include "JZNode.h"
#include "../JZModuleDefine.h"
#include "JZOpencv.h"

enum ModelNode
{
    Node_OpencvInit = Module_OpencvNode,
    Node_OpencvTemplate, 
};

class JZNodeOpencvInit : public JZNode
{
public:
    JZNodeOpencvInit();
    ~JZNodeOpencvInit();

    bool compiler(JZNodeCompiler* c, QString& error);
};

class JZNodeTemplateMatch : public JZNode
{
public:
    JZNodeTemplateMatch();
    ~JZNodeTemplateMatch();

    void setConfig(const JZTemplateConfig &config);
    JZTemplateConfig config();

    bool compiler(JZNodeCompiler* c, QString& error);
    void saveToStream(QDataStream& s) const;
    void loadFromStream(QDataStream& s);

protected:
    JZTemplateConfig m_config;
};

#endif