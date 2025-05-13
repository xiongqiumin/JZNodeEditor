#ifndef JZ_VISION_H_
#define JZ_VISION_H_

#include <opencv2/opencv.hpp>

using namespace cv;

Mat JZVisionCropImage(Mat mat,cv::Rect rc);
Mat JZVisionImageFlip(Mat mat,int h,int v);
void JZVisionImageMorphology();
void JZVisionImageRotate();
void JZVisionImageSplice();
void JZVisionPerspectiveTransform();
void JZVisionSkeleton();

void JZVisionBlobDetector();
void JZVisionBrightnessDetector();
void JZVisionColorIdentify();

void JZVisionShapeMatch();
void JZVisionTemplateMatch();

void JZVisionFindCircle();
void JZVisionFindLine();








#endif // ! JZ_VISION_H_
