#ifndef TEST_OPENCV_H_
#define TEST_OPENCV_H_

#include <QObject>
#include "../script/test_base.h"

class OpencvTest : public BaseTest
{
    Q_OBJECT

public:
    OpencvTest();

private slots:
    void testTemplateMatch();

protected:
   

};

void test_opencv(int argc, char *argv[]);

#endif
