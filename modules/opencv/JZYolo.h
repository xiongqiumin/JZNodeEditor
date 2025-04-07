#ifndef JZ_YOLO_H_
#define JZ_YOLO_H_

#include <QString>
#include <QRect>

class YoloResult
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

    bool loadNet(QString path);
    QList<YoloResult> forward(Mat mat);
};

#endif