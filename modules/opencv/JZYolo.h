#ifndef JZ_YOLO_H_
#define JZ_YOLO_H_

#include <QString>
#include <QRect>
#include <opencv2/opencv.hpp>
#include "JZNodeObject.h"

using namespace cv;

class JZYoloResult
{
public:
    QRect rect;
    QString name;
    double confidence;
};

class JZYolo
{
public:
    JZYolo();
    ~JZYolo();

    bool isVaild();
    bool loadNet();

    QString modelPath();
    void setModelPath(QString path);
    
    QList<JZYoloResult> forward(Mat mat);
    
    cv::dnn::Net m_net;
    QString m_modelPath;    
    QMap<int,QString> m_classList;
};

#endif