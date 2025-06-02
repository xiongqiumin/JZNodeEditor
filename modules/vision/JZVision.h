#ifndef JZ_VISION_H_
#define JZ_VISION_H_

#include <opencv2/opencv.hpp>
#include <QObject>

#include "JZShapeMatch.h"
#include "JZTemplateMatch.h"

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


#endif // ! JZ_VISION_H_
