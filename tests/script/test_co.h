#ifndef TEST_CO_H_
#define TEST_CO_H_

#include <QObject>
#include "test_base.h"

class CoTest : public BaseTest
{
    Q_OBJECT

public:
    CoTest();

private slots:
    void testLoop();
    void testModbus();

protected:

};

void test_co(int argc, char *argv[]);

#endif
