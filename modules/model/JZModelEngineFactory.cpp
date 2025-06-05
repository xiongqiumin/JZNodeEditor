#include <QLibrary>
#include <QDebug>
#include "JZModelEngineFactory.h"
#include "JZModelEngineOpencv.h"

// 实现单例模式
JZModelEngineFactory* JZModelEngineFactory::instance() 
{
    static JZModelEngineFactory inst;
    return &inst;
}

JZModelEngineFactory::JZModelEngineFactory()
{
    m_tensorRtCreator = nullptr;
}

JZModelEngineFactory::~JZModelEngineFactory()
{
}

// 加载TensorRT DLL
bool JZModelEngineFactory::loadTensorRtDll() 
{
    QLibrary tensorRtLib("TensorRTEngine");

    // 尝试加载DLL
    if (!tensorRtLib.load()) {
        qDebug() << "无法加载TensorRTEngine库:" << tensorRtLib.errorString();
        return false;
    }

    // 获取函数地址
    m_tensorRtCreator = (CreateTensorRtFunc)tensorRtLib.resolve("CreateTensorRtEngine");
    if (!m_tensorRtCreator) {
        qDebug() << "无法解析createTensorRtEngine函数";
        return false;
    }

    return true;
}

// 创建引擎
JZModelEngine* JZModelEngineFactory::createEngine(JZModelBackEnd type) {
    switch (type) {
    case Model_BackendCpu:
        return new JZModelEngineOpencv();
    case Model_BackendTensorRT:
        if (!m_tensorRtCreator)
        {
            if (!loadTensorRtDll())
                return nullptr;
        }
        return m_tensorRtCreator();
    default:
        std::cerr << "未知的引擎类型: " << type << std::endl;
        return nullptr;
    }

    return nullptr;
}