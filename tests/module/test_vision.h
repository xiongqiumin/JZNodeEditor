#ifndef TEST_VISION_H_
#define TEST_VISION_H_

#include <QObject>
#include "../script/test_base.h"

class VisionTest : public BaseTest
{
    Q_OBJECT

public:
    VisionTest();

private slots:
    void testTemplateMatch();
    void testBrightnessDetector();
    void testColorIdentify();
    void testVisonDemo();

protected:

};

void test_vision(int argc, char *argv[]);

#endif
