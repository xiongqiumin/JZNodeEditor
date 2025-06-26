#ifndef JZ_CAMERA_UTIL_TEST_H_
#define JZ_CAMERA_UTIL_TEST_H_

#include "JZScriptUnitTest.h"


//JZNodeCameraVistor
class JZNodeCameraVistor : public JZScriptUnitTestVistor
{
public:
    JZNodeCameraVistor();

    virtual void visitSelf(JZNode* node) override;
protected:

};

#endif