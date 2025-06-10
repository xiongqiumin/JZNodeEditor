#ifndef JZ_TENSOR_RT_H_
#define JZ_TENSOR_RT_H_

#include <QString>
#include <QDebug>
#include "modules/model/JZModelEngine.h"

// Qt风格的DLL导入导出宏
#ifdef TENSORRTENGINEDLL_EXPORTS
#define TENSORRTENGINEDLL_API Q_DECL_EXPORT
#else
#define TENSORRTENGINEDLL_API Q_DECL_IMPORT
#endif

class TensorRtEngineImpl;
class TensorRtEngine : public JZModelEngine
{
public:
    TensorRtEngine();
    ~TensorRtEngine();

    virtual bool isInit() override;
    virtual bool load(QString engine_path) override;
    virtual void destory() override;
    virtual cv::Mat forward(cv::Mat frame) override;

protected:
	TensorRtEngineImpl *d;
};

extern "C" TENSORRTENGINEDLL_API JZModelEngine *CreateTensorRtEngine();


#endif