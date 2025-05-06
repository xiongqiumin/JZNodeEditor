#ifndef JZ_YOLO_H_
#define JZ_YOLO_H_

#include <QString>
#include <QRect>
#include <opencv2/opencv.hpp>
#include "JZNodeObject.h"
#include "JZModel.h"

using namespace cv;

class JZYoloResult
{
public:
    QRect rect;
    QString name;
    double confidence;
};

class JZYolo : public JZModel
{
public:
    JZYolo();
    ~JZYolo();

    bool isVaild();
    virtual bool loadNet(QString path) override;    
    QList<JZYoloResult> forward(Mat mat);
    
    cv::dnn::Net m_net;     
    QMap<int,QString> m_classList;
};

#endif