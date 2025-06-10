#ifndef JZ_MODEL_ENGINE_ONNX_IMPL_H
#define JZ_MODEL_ENGINE_ONNX_IMPL_H
#include <QString>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

#include "onnxruntime_c_api.h"

// 实现类定义
class JZModelEngineOnnxImpl
{
public:
    JZModelEngineOnnxImpl();
    ~JZModelEngineOnnxImpl();

    bool loadModel(const QString& modelPath, QString* errorMessage = nullptr);
    cv::Mat forward(cv::Mat mat);
    
    QStringList getOutputNames() const;
    bool isModelLoaded() const;
    
private:
    // 初始化输入输出信息
    void initializeIOInfo();

    // 释放会话资源
    void releaseSession();        
    // 释放所有资源
    void releaseResources();        
    // 错误处理
    void throwOnError(OrtStatus* status, const std::string& message);
    
    
    mutable QMutex mutex;
    const OrtApi* api;
    OrtEnv* env;
    OrtSession* m_session;
    bool modelLoaded;
    OrtAllocator *m_allocator;

    // 模型输入信息
    std::string m_inputName;

    // 模型输出信息
    std::vector<std::string> m_outputNames;
};

#endif