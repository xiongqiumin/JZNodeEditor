#include "JZModelEngineOnnx.h"
#include <QString>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include "JZModelEngineOnnxImpl.h"

JZModelEngineOnnxImpl::JZModelEngineOnnxImpl()
{
    api = nullptr;
    env = nullptr;
    m_session = nullptr;
    modelLoaded = false;
    m_allocator = nullptr;

    // 获取全局 OrtApi 实例
    api = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    if (!api) {
        throw std::runtime_error("Failed to get ONNX Runtime API");
    }

    // 初始化环境
    OrtStatus* status = api->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "JZModelEngineOnnx", &env);
    if (status != NULL) {
        throwOnError(status, "Failed to create ONNX Runtime environment");
    }
    api->GetAllocatorWithDefaultOptions(&m_allocator);
}

JZModelEngineOnnxImpl::~JZModelEngineOnnxImpl() 
{
    releaseResources();
}

bool JZModelEngineOnnxImpl::loadModel(const QString& modelPath, QString* errorMessage) {
    QMutexLocker locker(&mutex);

    try {            
        // 创建会话选项
        OrtSessionOptions* session_options;
        OrtStatus* status = api->CreateSessionOptions(&session_options);
        if (status != NULL) {
            throwOnError(status, "Failed to create m_session options");
        }

        // 设置会话选项

        // 创建会话
        std::wstring model_path = modelPath.toStdWString();            
        status = api->CreateSession(env, model_path.c_str(), session_options, &m_session);
        api->ReleaseSessionOptions(session_options);

        if (status != NULL) {
            throwOnError(status, "Failed to create m_session from model");
        }

        // 获取输入和输出信息
        initializeIOInfo();
        modelLoaded = true;
        return true;
    }
    catch (const std::exception& e) {
        if (errorMessage) {
            *errorMessage = QString::fromStdString(e.what());
        }
        return false;
    }
}

cv::Mat JZModelEngineOnnxImpl::forward(cv::Mat mat) 
{
    QMutexLocker locker(&mutex);        

    // 创建输入张量
    OrtMemoryInfo* memory_info;
    OrtStatus* status = api->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);
    if (status != NULL) {
        throwOnError(status, "Failed to create CPU memory info");
    }

    QVector<qint64> inputDims;
    for (int i = 0; i < mat.dims; i++)
        inputDims << mat.size[i];

    OrtValue* input_tensor = nullptr;
    status = api->CreateTensorWithDataAsOrtValue(
        memory_info,
        (void*)mat.data,
        mat.total() * sizeof(float),
        inputDims.data(),
        inputDims.size(),
        ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
        &input_tensor);

    api->ReleaseMemoryInfo(memory_info);
    if (status != NULL) {
        throwOnError(status, "Failed to create input tensor");
    }

    // 运行推理
    QVector<const char *> outputNames;
    for (int i = 0; i < m_outputNames.size(); i++)
        outputNames.push_back(m_outputNames[i].data());

    const char* input_names[] = { m_inputName.c_str() };
    OrtValue* output_tensor = nullptr;
    status = api->Run(m_session,
        NULL,
        input_names,
        (const OrtValue* const*)&input_tensor,
        1,
        outputNames.data(),
        outputNames.size(),
        &output_tensor);

    api->ReleaseValue(input_tensor);
    if (status != NULL) {
        throwOnError(status, "Failed to run inference");
    }
    
    OrtTensorTypeAndShapeInfo *tensor_info;
    api->GetTensorTypeAndShape(output_tensor,&tensor_info);

    std::vector<int> outputDims;

    size_t num_dims = 0;
    api->GetDimensionsCount(tensor_info, &num_dims);
    
    QVector<int64_t> q_output_dims((int)num_dims);
    api->GetDimensions(tensor_info, q_output_dims.data(), num_dims);
    api->ReleaseTensorTypeAndShapeInfo(tensor_info);

    for (int i = 0; i < q_output_dims.size(); i++)
        outputDims.push_back(q_output_dims[i]);
    
    float *out_ptr;
    api->GetTensorMutableData(output_tensor, (void**)&out_ptr);
    cv::Mat result = cv::Mat(outputDims, CV_32F, out_ptr).clone();
    api->ReleaseValue(output_tensor);    

    return result;
}

QStringList JZModelEngineOnnxImpl::getOutputNames() const {
    QMutexLocker locker(&mutex);
    QStringList names;
    for (const auto& name : m_outputNames) {
        names.append(QString::fromStdString(name));
    }
    return names;
}

bool JZModelEngineOnnxImpl::isModelLoaded() const {
    QMutexLocker locker(&mutex);
    return modelLoaded;
}


// 初始化输入输出信息
void JZModelEngineOnnxImpl::initializeIOInfo() {
    // 获取输入信息
    size_t num_inputs;
    OrtStatus* status = api->SessionGetInputCount(m_session, &num_inputs);
    if (status != NULL) {
        throwOnError(status, "Failed to get input count");
    }

    if (num_inputs == 0) {
        throw std::runtime_error("Model has no inputs");
    }

    // 获取第一个输入的名称
    char* input_name;
    status = api->SessionGetInputName(m_session, 0, m_allocator, &input_name);
    if (status != NULL) {
        throwOnError(status, "Failed to get input name");
    }
    m_inputName = input_name;
    
    // 获取输出信息
    size_t num_outputs;
    status = api->SessionGetOutputCount(m_session, &num_outputs);
    if (status != NULL) {
        throwOnError(status, "Failed to get output count");
    }

    m_outputNames.resize(num_outputs);
    for (size_t i = 0; i < num_outputs; ++i) {
        char* output_name;
        status = api->SessionGetOutputName(m_session, i, m_allocator, &output_name);
        if (status != NULL) {
            throwOnError(status, "Failed to get output name");
        }
        m_outputNames[i] = output_name;
    }
}

// 释放会话资源
void JZModelEngineOnnxImpl::releaseSession() {
    if (m_session) {
        api->ReleaseSession(m_session);
        m_session = nullptr;
        modelLoaded = false;
    }
}

// 释放所有资源
void JZModelEngineOnnxImpl::releaseResources() {
    releaseSession();
    if (env) {
        api->ReleaseEnv(env);
        env = nullptr;
    }
}

// 错误处理
void JZModelEngineOnnxImpl::throwOnError(OrtStatus* status, const std::string& message) {
    const char* msg = api->GetErrorMessage(status);
    std::string error_msg = message + ": " + msg;
    api->ReleaseStatus(status);
    throw std::runtime_error(error_msg);
}
