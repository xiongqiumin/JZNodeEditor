#include "JZModelEngineOpencv.h"

using namespace cv;

JZModelEngineOpencv::JZModelEngineOpencv()
{
}
JZModelEngineOpencv::~JZModelEngineOpencv()
{

}

 
bool JZModelEngineOpencv::isInit()
{
    return !m_net.empty();
}

bool JZModelEngineOpencv::load(QString modelPath)
{
    try {
        m_net = cv::dnn::readNet(modelPath.toLocal8Bit().data());

        //m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);  // 使用OpenCV的OpenCL实现
        //m_net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);    // 通用OpenCL设备
    }
    catch (std::exception& e)
    {
        return false;
    }
    return true;
}

void JZModelEngineOpencv::destory()
{
    m_net = cv::dnn::Net();
}

cv::Mat JZModelEngineOpencv::forward(cv::Mat frame)
{
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
    m_net.setInput(blob);
    cv::Mat preds = m_net.forward();
    return preds;
}