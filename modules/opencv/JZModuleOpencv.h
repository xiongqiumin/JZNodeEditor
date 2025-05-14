#ifndef JZ_MODULE_OPENCV_H_
#define JZ_MODULE_OPENCV_H_

#include "JZModule.h"
#include "../JZModuleDefine.h"
#include "CvMatAndQImage.h"
#include "JZOpencv.h"

class JZModuleOpencv: public JZModule
{
public:
    JZModuleOpencv();
    virtual ~JZModuleOpencv();

    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;      

protected:
    void registCvtEnum(JZScriptEnvironment *env);
};

#endif