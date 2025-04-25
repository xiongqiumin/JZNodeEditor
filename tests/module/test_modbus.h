#ifndef TEST_MODBUS_H_
#define TEST_MODBUS_H_

#include <QObject>
#include "../script/test_base.h"

class ModbusTest : public BaseTest
{
    Q_OBJECT

public:
    ModbusTest();

private slots:
    void testClient();

protected:

};

void test_modbus(int argc, char *argv[]);

#endif
