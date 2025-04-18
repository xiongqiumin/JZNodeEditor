#ifndef TEST_TX_H_
#define TEST_TX_H_

#include <QObject>
#include "test_base.h"

class TxTest : public BaseTest
{
    Q_OBJECT

public:
    TxTest();

private slots:
    void testLoop();

protected:

};

void test_tx(int argc, char *argv[]);

#endif
