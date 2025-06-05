#ifndef JZ_YOLO_H_
#define JZ_YOLO_H_

#include <QString>
#include <QRect>
#include <opencv2/opencv.hpp>
#include "JZNodeObject.h"
#include "JZModel.h"
#include "jzWidgets/JZImageLabel.h"
#include "JZModelEngine.h"

using namespace cv;

class JZModelYoloConfig : public JZModelConfig
{
public:
    JZModelYoloConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModelBackEnd backend;
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
    static QList<JZGraphic> toGraphics(const QList<JZYoloResult>& result);

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
    virtual void deinit() override;

    QList<JZYoloResult> forward(Mat mat);
    
protected:
    struct LetterboxResult
    {
        float scale;       // 缩放比例
        int pad_x;         // X方向填充量
        int pad_y;         // Y方向填充量
    };

    // 等比缩放并填充灰色边缘，同时返回缩放比例和填充偏移量
    cv::Mat makeLetterImage(const cv::Mat& img, cv::Size new_shape, LetterboxResult &box_result);
    cv::Mat normalizedImage(cv::Mat img);
    cv::Rect mapCoordinates(const cv::Rect& box, float scale, int pad_x, int pad_y);

    bool loadClassInfo(QString class_into);
  
    JZModelEnginePtr m_net;
    QMap<int,QString> m_classList;
};

#endif