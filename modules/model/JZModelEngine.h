#ifndef JZMODEL_ENGINE_H_
#define JZMODEL_ENGINE_H_

#include <QString>
#include <opencv2/opencv.hpp>
#include <QSharedPointer>

class JZModelEngine
{
public:
	virtual ~JZModelEngine() = default;
	
	virtual bool isInit() = 0;
	virtual bool load(QString path) = 0;
	virtual void destory() = 0;
	virtual cv::Mat forward(cv::Mat frame) = 0;
	
};
typedef QSharedPointer<JZModelEngine> JZModelEnginePtr;

#endif