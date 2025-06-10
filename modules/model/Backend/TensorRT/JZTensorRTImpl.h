#ifndef JZ_TENSOR_IMPL_RT_H_
#define JZ_TENSOR_IMPL_RT_H_

#include <QString>
#include <QDebug>
#include "NvInfer.h"
#include "NvOnnxParser.h"
#include "cuda_runtime_api.h"
#include "opencv2/opencv.hpp"

class TRTLogger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        qDebug() << "TensorRT Logger: " << msg;
    }
};

template<typename T>
struct TRTDeleter {
    void operator()(T* obj) const {
        if (obj) {
            obj->destroy();
        }
    }
};

template<typename T>
using TRTUniquePtr = std::unique_ptr<T, TRTDeleter<T>>;

class TensorRtEngineImpl
{
public:
    TensorRtEngineImpl();
    ~TensorRtEngineImpl();

    bool isInit();
    bool load(QString engine_path);
    void destory();
    cv::Mat forward(cv::Mat frame);

    TRTUniquePtr<nvinfer1::IRuntime> m_runtime;
    TRTUniquePtr<nvinfer1::ICudaEngine> m_engine;
    TRTUniquePtr<nvinfer1::IExecutionContext> m_context;

    int m_inputSize;
    int m_outputSize;
    void *m_cudaInBuffer;
    void *m_cudaOutBuffer;
    cudaStream_t m_stream;
    TRTLogger m_log;
};


#endif