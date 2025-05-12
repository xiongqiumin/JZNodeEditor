#ifndef TEST_RUNTIME_H_
#define TEST_RUNTIME_H_

#include <QObject>
#include "test_base.h"

class RuntimeTest : public BaseTest
{
    Q_OBJECT

public:
    RuntimeTest();

private slots:
    void testFormatString();
    void testFormatBinary();

protected:

};

void test_runtime(int argc, char *argv[]);

#endif
