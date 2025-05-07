#include "JZOpencv.h"

using namespace cv;

//JZTemplateConfig
JZTemplateConfig::JZTemplateConfig()
{
	confidence = 0.95;
}

QDataStream& operator<<(QDataStream& s, const JZTemplateConfig& config)
{
	s << config.templatePath;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZTemplateConfig& config)
{
	s >> config.templatePath;
	return s;
}

void JZTemplate::init(const JZTemplateConfig& config)
{
	m_config = config;
	m_templ = imread(config.templatePath.toLocal8Bit().data(), IMREAD_GRAYSCALE);
}

QRect JZTemplate::match(cv::Mat img)
{
	// 创建结果矩阵
	Mat result;
	int result_cols = img.cols - m_templ.cols + 1;
	int result_rows = img.rows - m_templ.rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);

	// 进行模板匹配
	matchTemplate(img, m_templ, result, TM_CCOEFF_NORMED);

	// 找到匹配结果中的最大值和最小值及其位置
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

	if (maxVal < m_config.confidence)
		return QRect();

	return toQRect(cv::Rect(maxLoc, Point(maxLoc.x + m_templ.cols, maxLoc.y + m_templ.rows)));
}

QRect JZOpencvTemplateMatch(JZTemplate* temp, cv::Mat image)
{
	return QRect();
}