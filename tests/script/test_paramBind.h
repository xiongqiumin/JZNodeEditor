#ifndef TEST_PARAM_BIND_H_
#define TEST_PARAM_BIND_H_

#include <QObject>
#include <QElapsedTimer>
#include "test_base.h"

class ParamBindTest : public BaseTest
{
    Q_OBJECT

public:
    ParamBindTest();

private slots:
    void testSample();

protected:
};

void test_paramBind(int argc, char* argv[]);

#endif