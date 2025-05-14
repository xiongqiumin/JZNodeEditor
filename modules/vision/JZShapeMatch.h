#ifndef JZ_SHAPE_MATCH
#define JZ_SHAPE_MATCH

#include <QObject>
#include <opencv2/opencv.hpp>

//JZShapeConfig
class JZShapeConfig
{
public:
    JZShapeConfig();

    QString templatePath;
    double confidence;
};
QDataStream& operator<<(QDataStream& s, const JZShapeConfig& config);
QDataStream& operator >> (QDataStream& s, JZShapeConfig& config);

//JZShapeMatch
class JZShapeMatch : public QObject
{
    Q_OBJECT

public:
    void init(const JZShapeConfig& config);
    QRect match(cv::Mat image);

protected:
    cv::Mat m_templ;
    JZShapeConfig m_config;
};


#endif