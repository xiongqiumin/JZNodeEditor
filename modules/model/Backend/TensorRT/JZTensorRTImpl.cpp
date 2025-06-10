#include <QFile>
#include "JZTensorRTImpl.h"

#pragma comment(lib, "E:/libs/CUDA/TensorRT-8.2.5.1/lib/nvinfer.lib")
#pragma comment(lib, "E:/libs/CUDA/TensorRT-8.2.5.1/lib/nvonnxparser.lib")
#pragma comment(lib, "E:/libs/CUDA/v11.4/lib/x64/cudart.lib")

using namespace cv;

TensorRtEngineImpl::TensorRtEngineImpl()
{
    m_stream = nullptr;
    m_cudaInBuffer = nullptr;
    m_cudaOutBuffer = nullptr;
    m_inputSize = 0;
    m_outputSize = 0;
}

TensorRtEngineImpl::~TensorRtEngineImpl()
{
    destory();    
}

bool TensorRtEngineImpl::isInit()
{    
    return m_engine.get();
}

bool TensorRtEngineImpl::load(QString engine_path)
{
    TRTUniquePtr<nvinfer1::IRuntime> runtime{ nvinfer1::createInferRuntime(m_log) };
    if (!runtime) {
        std::cerr << "Failed to create TensorRT runtime!" << std::endl;
        return false;
    }

    QFile file(engine_path);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray engineData = file.readAll();
    file.close();

    TRTUniquePtr<nvinfer1::ICudaEngine> engine;
    engine.reset(runtime->deserializeCudaEngine(engineData.data(), engineData.size(), nullptr));
    if (!engine)
        return false;

    // 创建执行上下文
    TRTUniquePtr<nvinfer1::IExecutionContext> context{ engine->createExecutionContext() };
    if (!context) {
        std::cerr << "Failed to create TensorRT execution context!" << std::endl;
        return false;
    }

    // 获取输入输出信息
    int inputIndex = engine->getBindingIndex("images");
    int outputIndex = engine->getBindingIndex("output0");

    // 获取输入尺寸
    nvinfer1::Dims inputDims = engine->getBindingDimensions(inputIndex);
    int inputC = inputDims.d[1];
    int inputH = inputDims.d[2];
    int inputW = inputDims.d[3];

    // 计算输入和输出大小
    size_t inputSize = inputC * inputH * inputW * sizeof(float);
    nvinfer1::Dims outputDims = engine->getBindingDimensions(outputIndex);
    size_t outputSize = 1;
    for (int i = 0; i < outputDims.nbDims; ++i) {
        outputSize *= outputDims.d[i];
    }
    outputSize *= sizeof(float);
    
    m_inputSize = (int)inputSize;
    m_outputSize = (int)outputSize;
    cudaStreamCreate(&m_stream);

    // 分配GPU内存    
    cudaMalloc(&m_cudaInBuffer, inputSize);
    cudaMalloc(&m_cudaOutBuffer, outputSize);

    m_runtime = std::move(runtime);
    m_engine = std::move(engine);
    m_context = std::move(context);
    return true;
}

void TensorRtEngineImpl::destory()
{
    if (m_cudaInBuffer)
    {
        cudaFree(m_cudaInBuffer);
        m_cudaInBuffer = nullptr;
    }
    if (m_cudaOutBuffer)
    {
        cudaFree(m_cudaOutBuffer);
        m_cudaOutBuffer = nullptr;
    }    
    if (m_stream)
    {
        cudaStreamDestroy(m_stream);
        m_stream = nullptr;
    }

    m_context.reset();
    m_engine.reset();
    m_runtime.reset();
    m_inputSize = 0;
    m_outputSize = 0;
}

cv::Mat TensorRtEngineImpl::forward(cv::Mat frame_input)
{
    Q_ASSERT(m_inputSize == frame_input.total() * frame_input.elemSize());
	
    int inputH = 640;
    int inputW = 640;
    // 转为CHW格式
    Mat frame(1, m_inputSize, CV_32F);
    float* ptr = (float*)frame.data;

    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < inputH; ++h) {
            for (int w = 0; w < inputW; ++w) {
                ptr[c * inputH * inputW + h * inputW + w] = frame_input.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    int sizes[] = { 1, 84, 8400 };
    cv::Mat ret(3, sizes, CV_32F);
    int inputSize = m_inputSize;
    int outputSize = m_outputSize;

    // 将输入数据传输到GPU
    cudaMemcpyAsync(m_cudaInBuffer, frame.data, inputSize,
        cudaMemcpyHostToDevice, m_stream);

    // 执行推理
    void* deviceBuffers[2];
    deviceBuffers[0] = m_cudaInBuffer;
    deviceBuffers[1] = m_cudaOutBuffer;
    m_context->enqueueV2(deviceBuffers, m_stream, nullptr);

    // 将输出数据传输到CPU
    cudaMemcpyAsync(ret.data, m_cudaOutBuffer, outputSize,
        cudaMemcpyDeviceToHost, m_stream);

    // 等待流完成
    cudaStreamSynchronize(m_stream);

    return ret;
}