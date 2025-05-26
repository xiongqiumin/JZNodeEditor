#ifndef JZ_YOLO_H_
#define JZ_YOLO_H_

#include <QString>
#include <QRect>
#include <opencv2/opencv.hpp>
#include "JZNodeObject.h"
#include "JZModel.h"
#include "jzWidgets/JZImageLabel.h"

using namespace cv;

class JZModelYoloConfig : public JZModelConfig
{
public:
    JZModelYoloConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString modelPath;
    QString idPath;
    double confThreshold;
    double nmsThreshold;
};
QDataStream& operator<<(QDataStream& s, const JZModelYoloConfig& param);
QDataStream& operator>>(QDataStream& s, JZModelYoloConfig& param);

class JZYoloResult
{
public:
    QList<JZGraphic> toGraphics(const QList<JZYoloResult>& result);

    QRect rect;
    int id;
    QString name;
    double confidence;
};

class JZYolo : public JZModel
{
public:
    JZYolo();
    ~JZYolo();

    virtual bool isInit() override;
    virtual bool init() override;
    QList<JZYoloResult> forward(Mat mat);
    
protected:
    bool loadClassInfo(QString class_into);

    cv::dnn::Net m_net;     
    QMap<int,QString> m_classList;
};

#endif