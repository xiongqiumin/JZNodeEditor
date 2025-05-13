#ifndef CV_TO_QT_H_
#define CV_TO_QT_H_

#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QPointF>
#include "CvMatAndQImage.h"

QSize toQSize(const cv::Size& cvRect);
QSizeF toQSizeF(const cv::Size2d& cvRectF);
cv::Size fromQSize(const QSize& qRect);
cv::Size2d fromQSizeF(const QSizeF& qRectF);

QRect toQRect(const cv::Rect& cvRect);
QRectF toQRectF(const cv::Rect2d& cvRectF);
cv::Rect fromQRect(const QRect& qRect);
cv::Rect2d fromQRectF(const QRectF& qRectF);

QPoint toQPoint(const cv::Point& cvPoint);
cv::Point fromQPoint(const QPoint& qPoint);
QPointF toQPointF(const cv::Point2d& cvPointF);
cv::Point2d fromQPointF(const QPointF& qPointF);


#endif