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
    void testException();
    void testABSwitch();
    void testThread();

protected:

};

void test_co(QStringList testcase = QStringList());

#endif
