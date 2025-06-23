#ifndef JZ_VISION_NODE_INFO_H_
#define JZ_VISION_NODE_INFO_H_

#include <QString>
#include "jzWidgets/JZImageGraphic.h"
#include "JZNode.h"
#include "modules/vision/JZVision.h"

class JZVisionNodeInfo
{
public:
    JZVisionNodeInfo();

    QString name;
    JZNode *node;
};

struct ImageResult
{
    cv::Mat mat;
    QList<JZGraphic> graphList;
};

struct NodeResult
{
    QList<ImageResult> outputImage;
    QString error;
};

class JZVisionRuntimeResult
{
public:
    JZVisionRuntimeResult();
    void clear();

    QMap<int, NodeResult> nodeResult;
};

#endif