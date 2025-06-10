#ifndef JZ_VISION_H_
#define JZ_VISION_H_

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/wechat_qrcode.hpp>

#include "JZShapeMatch.h"
#include "JZTemplateMatch.h"
#include "modules/model/PaddleOCR/JZPaddleOCR.h"

using namespace cv;

Mat JZVisionImageThreshold(Mat mat, QRect rc);
Mat JZVisionImageCrop(Mat mat,QRect rc);
Mat JZVisionImageFlip(Mat mat,bool h,bool v);
void JZVisionImageMorphology();
Mat JZVisionPerspectiveTransform(Mat mat,QRect from, QRect to);
Mat JZVisionSkeleton(Mat src, int intera);

std::vector<cv::KeyPoint> JZVisionBlobDetector(Mat src, cv::SimpleBlobDetector::Params param);

struct BrightnessDetectorResult
{
    double cast;
    double da;
};

BrightnessDetectorResult JZVisionBrightnessDetector(Mat gary_img);
double JZVisionColorIdentify(Mat src_ori, Mat src_mat);

void JZVisionFindCircle(Mat in);
void JZVisionFindLine(Mat in);

class JZBarCode : public QObject
{
public:

};

class JZQRCode : public QObject
{
public:

};

void JZVisionBarCode(Mat in);
void JZVisionQrCode(Mat in);
void JZVisionOCR(Mat in);

#endif // ! JZ_VISION_H_
