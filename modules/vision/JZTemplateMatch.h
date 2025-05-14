#ifndef JZ_TEMPLATE_MATCH
#define JZ_TEMPLATE_MATCH

#include <QObject>
#include <QRect>
#include <opencv2/opencv.hpp>

//JZTemplateConfig
class JZTemplateConfig
{
public:
    JZTemplateConfig();

    QString templatePath;
    double confidence;
};
QDataStream& operator<<(QDataStream& s, const JZTemplateConfig& config);
QDataStream& operator >> (QDataStream& s, JZTemplateConfig& config);

//JZTemplateMatch
class JZTemplateMatch : public QObject
{
    Q_OBJECT

public:
    void init(const JZTemplateConfig& config);
    QRect match(cv::Mat image);

protected:
    cv::Mat m_templ;
    JZTemplateConfig m_config;
};

void JZTemplateMatchInit(QObject *obj,QString name,QByteArray buffer);

#endif