#ifndef TEST_MODEL_H_
#define TEST_MODEL_H_

#include <QObject>
#include "../script/test_base.h"

class ModelTest : public BaseTest
{
    Q_OBJECT

public:
    ModelTest();

private slots:
    void testYolo();

protected:
    Benchmark m_benchmark;

};

void test_model(int argc, char *argv[]);

#endif // ! TEST_CAMERA_H_
