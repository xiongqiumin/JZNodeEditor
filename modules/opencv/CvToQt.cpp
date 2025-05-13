#include "CvToQt.h"

QSize toQSize(const cv::Size& cv_size)
{
    return QSize(cv_size.width, cv_size.height);
}

QSizeF toQSizeF(const cv::Size2d& cv_size)
{
    return QSizeF(cv_size.width, cv_size.height);
}

cv::Size fromQSize(const QSize& q_size)
{
    return cv::Size(q_size.width(), q_size.height());
}

cv::Size2d fromQSizeF(const QSizeF& q_size)
{
    return cv::Size2d(q_size.width(), q_size.height());
}

QRect toQRect(const cv::Rect& cvRect) {
    return QRect(cvRect.x, cvRect.y, cvRect.width, cvRect.height);
}

QRectF toQRectF(const cv::Rect2d& cvRectF) {
    return QRectF(cvRectF.x, cvRectF.y, cvRectF.width, cvRectF.height);
}

cv::Rect fromQRect(const QRect& qRect) {
    return cv::Rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());
}

cv::Rect2d fromQRectF(const QRectF& qRectF) {
    return cv::Rect2d(qRectF.x(), qRectF.y(), qRectF.width(), qRectF.height());
}

QPoint toQPoint(const cv::Point& cvPoint) {
    return QPoint(cvPoint.x, cvPoint.y);
}

cv::Point fromQPoint(const QPoint& qPoint) {
    return cv::Point(qPoint.x(), qPoint.y());
}

QPointF toQPointF(const cv::Point2d& cvPointF) {
    return QPointF(cvPointF.x, cvPointF.y);
}

cv::Point2d fromQPointF(const QPointF& qPointF) {
    return cv::Point2d(qPointF.x(), qPointF.y());
}