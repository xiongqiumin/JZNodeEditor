#pragma once

#include "JZModel.h"
#include "JZModelEngine.h"

typedef JZModelEngine* (*CreateTensorRtFunc)();

// 模型引擎工厂类 - 用于创建不同类型的推理引擎
class JZModelEngineFactory 
{
public:
    static JZModelEngineFactory *instance();

    JZModelEngine* createEngine(JZModelBackEnd type);

protected:
    JZModelEngineFactory();
    ~JZModelEngineFactory();

    bool loadTensorRtDll();

    CreateTensorRtFunc m_tensorRtCreator;
};