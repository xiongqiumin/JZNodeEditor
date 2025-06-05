#ifndef JZMODEL_ENGINE_OPENCV_H_
#define JZMODEL_ENGINE_OPENCV_H_

#include "JZModelEngine.h"

class JZModelEngineOpencv : public JZModelEngine
{
public:
	JZModelEngineOpencv();
	virtual ~JZModelEngineOpencv();

	virtual bool isInit() override;
	virtual bool load(QString path) override;
	virtual void destory() override;
	virtual cv::Mat forward(cv::Mat frame) override;

protected:
	cv::dnn::Net m_net;
};

#endif // !JZMODEL_ENGINE_OPENCV_H_
