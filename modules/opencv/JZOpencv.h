#ifndef JZ_OPENCV_H_
#define JZ_OPENCV_H_

#include <QString>
#include <opencv2/opencv.hpp>
#include "CvToQt.h"

//JZTemplateConfig
class JZTemplateConfig
{
public:
    JZTemplateConfig();

    QString templatePath;
    double confidence;
};
QDataStream& operator<<(QDataStream& s, const JZTemplateConfig& config);
QDataStream& operator>>(QDataStream& s, JZTemplateConfig& config);

//JZTemplate
class JZTemplate : public QObject
{
    Q_OBJECT

public:
    void init(const JZTemplateConfig& config);
    QRect match(cv::Mat image);

protected:
    cv::Mat m_templ;
    JZTemplateConfig m_config;
};

#endif