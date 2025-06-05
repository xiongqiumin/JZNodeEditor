#include <QFile>
#include "JZTensorRT.h"
#include "JZTensorRTImpl.h"

TensorRtEngine::TensorRtEngine()
{
    d = new JZTensorRTImpl();
}

TensorRtEngine::~TensorRtEngine()
{
    delete d;    
}

bool TensorRtEngine::isInit()
{    
    return m_engine.get();
}

bool TensorRtEngine::load(QString engine_path)
{
    return d->load(engine_path);
}

void TensorRtEngine::destory()
{
    return d->destory();
}

cv::Mat TensorRtEngine::forward(cv::Mat frame)
{
    return d->forward(frame);
}

JZModelEngine *CreateTensorRtEngine()
{
	return new TensorRtEngine();
}